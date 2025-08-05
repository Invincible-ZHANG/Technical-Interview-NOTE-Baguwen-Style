# resnet_train.py

import torch
import torchvision
import torchvision.transforms as transforms
import torch.nn as nn
import torch.optim as optim
from torchvision.models import resnet18

# 1. 数据预处理（CIFAR10 → ResNet 输入 224x224）
transform = transforms.Compose([
    transforms.Resize(224),
    transforms.ToTensor(),
    transforms.Normalize((0.5,), (0.5,))
])

trainset = torchvision.datasets.CIFAR10(root='./data', train=True, download=True, transform=transform)
testset  = torchvision.datasets.CIFAR10(root='./data', train=False, download=True, transform=transform)

trainloader = torch.utils.data.DataLoader(trainset, batch_size=64, shuffle=True)
testloader  = torch.utils.data.DataLoader(testset, batch_size=1000)

# 2. 加载预训练 ResNet18 模型
model = resnet18(pretrained=True)
model.fc = nn.Linear(model.fc.in_features, 10)  # 替换最后一层输出10类

# 3. 损失函数与优化器
criterion = nn.CrossEntropyLoss()
optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.9)

# 4. Finetune
for epoch in range(5):
    model.train()
    for inputs, labels in trainloader:
        outputs = model(inputs)
        loss = criterion(outputs, labels)

        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
    print(f"[Epoch {epoch}] Loss = {loss.item():.4f}")

# 5. 测试集评估
correct = 0
total = 0
model.eval()
with torch.no_grad():
    for x, y in testloader:
        outputs = model(x)
        pred = outputs.argmax(dim=1)
        correct += (pred == y).sum().item()
        total += y.size(0)

print(f"[ResNet] CIFAR10 Accuracy: {100 * correct / total:.2f}%")
