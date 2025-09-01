---
title: 谈谈Dantzig求解器
date: 2025-08-29
excerpt: "对于现在VEROSIM已经实现的Danzig谈一下自己的理解。"
layout: note
---

> 对于现在VEROSIM已经实现的Danzig谈一下自己的理解。


## 整体流程

1. **装配 LCP**

   * $A = J M^{-1}J^\top$
   * $b = J(v_{\text{old}} + M^{-1}\Delta t f_{\text{ext}}) + \text{Baumgarte}$
   * lo/hi（切向：线性耦合到法向）、findex（摩擦块）
2. **塞进 Dantzig 对象**

   * `setValuesInMatrix(A)` / `setValuesInRightSide(-b)`
   * `setLowVector(lo)` / `setHighVector(hi)`
   * `setFrictionIndex(findex)` / `setNumberFrictionConstraints(k)`
~~~
//Set the frictionNormalIndices, lambdaLow, lambdaHigh, numberFrictionConstraints and addFriction in lcp solver
		myLcp->setFrictionIndex(frictionNormalIndices);
		myLcp->setLowVector(lambdaLow);
		myLcp->setHighVector(lambdaHigh);
		myLcp->setNumberFrictionConstraints(numberFrictionConstraints);
		myLcp->setAddFriction(addFriction);
~~~

3. **调用 ODE**

   * `dxSolveLCP(..., A, pairsbx(b/x), schlupf(w), nub, pairslh(lo/hi), findex)`
   * 枢轴法在内部**维持盒约束与摩擦块可行性**（= 隐式“投到金字塔上”）

往上一层一层找：dxSolveLCP -> dxSolveLCP_Generic

~~~
void dxSolveLCP ( ... , unsigned n, dReal *A, dReal pairsbx[PBX__MAX],
    dReal *outer_w, unsigned nub, dReal pairslh[PLH__MAX], int *findex)
{
    ...
    dxSolveLCP_Generic(memarena, n, A, pairsbx, outer_w, nub, pairslh, findex);
}
~~~



4. **拿解回代**

   * `lambda = getLambda()`
   * $v_{\text{new}} = M^{-1}J^{\mathsf T}\lambda + M^{-1}\Delta t f_{\text{ext}}$






## 头文件 RBDLcpDantzig.h

* VSLibRBDynMathMixedLCP：

**“混合线性互补问题（Mixed LCP, MLCP）求解器”的抽象基类接口**，要解的是（含等式/无界与盒约束的）MLCP。

* `getLambda()`: 返回解向量 $x$（通常就是约束乘子 $\lambda$）。
* `solve()`: 真正“开算”的入口；返回是否成功。
* `setValuesInMatrix(const VSM::MatrixNxM& values)`: 设置 $A$ 矩阵（一般是 $J M^{-1} J^\top$）。
* `setValuesInRightSide(const VSM::VectorN& values)`: 设置右端 $b$。
* `setLowVector(...) / setHighVector(...)`: 设置盒约束上下界 $\ell, h$。
* `setFrictionIndex(const VSM::VectorNDynamicTemplate<int>&)`: 设置 **findex**（将切向变量“指向”其法向变量，实现摩擦块耦合，用于金字塔摩擦）。
* `setAddFriction(const VSM::VectorNDynamic&)`: 可选的附加摩擦参数（旧版 ODE 接口会用到）。
* `validSolution() const`: 校验解是否满足互补性/边界（这里只声明，默认实现通常在 `.cpp`）。

> 调用顺序通常是：
> `setValuesInMatrix(A)` → `setValuesInRightSide(b)` → `setLow/High(ℓ/h)` → `setFrictionIndex(findex)` → `setNumberFrictionConstraints(k)` → `solve()` → `getLambda()`。



**对 ODE（Open Dynamics Engine） 的 Dantzig LCP 例程的 OO 包装。**

把 ODE 里“用 Dantzig 枢轴法求解 LCP 的那套纯 C 接口”，用面向对象（OO）的方式“包一层壳”，变成一个好用、可替换、可扩展的 C++ 类。

怎么理解：定义了一个抽象基类 RBMMixedLCP（统一协议），再写一个具体实现类 RBDLcpDantzig 去“适配” ODE，通过这种“包一层”，上层完全不知道 ODE 的存在：想换成 APGD、PGS，只要再写一个继承 RBMMixedLCP 的类即可，实现同样的虚函数，无缝热插拔。



~~~
    inline VSM::VectorN& getLambda() const
      {
         return *x;
      };

      inline VSM::VectorN& ref_w() const
      {
         return *w;
      };
~~~



这两个函数都是“**把求解器内部向量暴露给外部读取/检查**”的快捷入口——一个拿**解 $\lambda$**，一个拿**互补余量 $w$**。对应 LCP/MLCP 的标准记号：

* $x \equiv \lambda$：约束乘子（你最后要回代到速度里的量）
* $w = A\,x - b$：互补变量/“余量”（用来检查互补性与激活边界）




~~~
void ldltDecomposition(
         const VSLibRBDynMath::RBMMatrix& A,
         //      const VSLibRBDynMath::RBMIndexSet& C,
         VSLibRBDynMath::RBMMatrix& L);

     
      virtual inline void setValuesInMatrix(
         const VSM::MatrixNxM& values)
     
~~~

矩阵装配/分解相关


ldltDecomposition：提供一个数值分解工具（LDLᵀ），为快速解线性子问题/预条件做准备；

setValuesInMatrix：把“常规矩阵”拷贝成 ODE 期望的行对齐内存布局，是把你的$𝐴$喂给 ODE LCP 例程前的必要装配步骤。



ODE 的通用 LCP 解算入口 dxSolveLCP_Generic：

```

        if (!hit_first_friction_index && findex && findex[i] >= 0) {
            // un-permute x into delta_w, which is not being used at the moment
            for (unsigned j = 0; j < n; ++j) delta_w[p[j]] = (pairsbx + (sizeint)j * PBX__MAX)[PBX_X];

            // set lo and hi values
            for (unsigned k = i; k < n; ++k) {
                dReal *currlh = pairslh + (sizeint)k * PLH__MAX;
                dReal wfk = delta_w[findex[k]];
                if (wfk == 0) {
                    currlh[PLH_HI] = 0;
                    currlh[PLH_LO] = 0;
                }
                else {
                    currlh[PLH_HI] = dFabs (currlh[PLH_HI] * wfk);
                    currlh[PLH_LO] = -currlh[PLH_HI];
                }
            }
            hit_first_friction_index = true;
        }
```




## Reference


    RBDLcpDantzig.h

    RBDLcpDantzig.cpp

