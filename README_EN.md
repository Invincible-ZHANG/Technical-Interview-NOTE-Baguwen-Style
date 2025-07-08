[中文](./README.md) | English
# Technical Interview Notes 📚

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE) [![GitHub Pages](https://img.shields.io/badge/Pages-📖%20Online-blue)](https://invincible-zhang.github.io/Technical-Interview-NOTE-Baguwen-Style/) [![Languages](https://img.shields.io/badge/Langs-C%2B%2B%20%7C%20Python%20%7C%20CUDA-lightgrey)]()

> Embedded Systems · Software Engineering · GPU/CUDA · Algorithms

---

## Table of Contents

* [Introduction](#introduction)  
* [Quick Start](#quick-start)  
* [Key Features](#key-features)  
* [Project Structure](#project-structure)  
* [Examples](#examples)  
* [Customization & Deployment](#customization--deployment)  
* [Contributing](#contributing)  
* [License](#license)

---

## Introduction

**Technical Interview Notes** is a comprehensive knowledge base covering Embedded Systems, Multibody Dynamics, Solver/Optimization Algorithms, and GPU/CUDA parallel computing. Written in **Markdown** and automatically deployed via **Jekyll + GitHub Pages**, it supports local preview and CI/CD.

---

## Quick Start

```bash
# Clone the repository
git clone https://github.com/Invincible-ZHANG/Technical-Interview-NOTE-Baguwen-Style.git
cd Technical-Interview-NOTE-Baguwen-Style

# Serve locally with Jekyll
cd docs
bundle install        # Install dependencies (first run only)
bundle exec jekyll serve --port 4000
# Open your browser at http://localhost:4000
````

---

## Key Features

* 🚀 **Domain Coverage**: Embedded Development · Software Engineering · GPU/CUDA Acceleration · Algorithms & Numerical Methods
* ✍️ **Markdown Notes**: Consistent format, YAML Front Matter, auto-generated navigation and indexes
* 🔄 **Jekyll Build**: `docs/` folder + Minima theme → static website
* 🔧 **GitHub Actions**: CI/CD pipeline for automated build & deployment to GitHub Pages
* 📂 **Jekyll Collections**: Organize content by modules (`embedded`, `software`, `gpu_cuda`, `algorithms`)

---

## Project Structure

```text
├── .github/               # GitHub Actions workflows & templates
├── docs/                  # Jekyll source & GitHub Pages root
│   ├── _config.yml        # Jekyll configuration
│   ├── index.md           # Site homepage
│   ├── assets/            # Static assets (CSS, images)
│   ├── _embedded/         # Embedded systems collection
│   ├── _software/         # Software engineering collection
│   ├── _gpu_cuda/         # GPU/CUDA collection
│   ├── _algorithms/       # Algorithms collection
│   ├── embedded/          # Embedded index pages
│   ├── software/          # Software index pages
│   ├── gpu-cuda/          # GPU/CUDA index pages
│   └── algorithms/        # Algorithms index pages
├── examples/              # Sample code
├── notes/                 # Original notes (optional sync)
├── scripts/               # Build & validation scripts
├── .gitignore             # Ignore patterns
├── LICENSE                # MIT License
└── README_EN.md           # English project overview
```

---

## Examples

* **Live Site**: [GitHub Pages](https://invincible-zhang.github.io/Technical-Interview-NOTE-Baguwen-Style/)
* **Sample Notes**: Embedded [RTOS Basics](/embedded/rtos-basics.html), GPU/CUDA [Parallel Model](/gpu-cuda/cuda-intro.html)

---

## Customization & Deployment

1. Edit `docs/_config.yml` to change themes or enable plugins.
2. Add or update Markdown files in the appropriate collection folders (`docs/_embedded`, `docs/_software`, etc.).
3. Configure `.github/workflows/deploy.yml` for automated builds (refer to the provided template).
4. Push to the `main` branch; GitHub Actions will automatically rebuild and deploy.

---

## Contributing

We welcome your feedback and contributions:

1. Fork this repository
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Commit your changes and push: `git push origin feature/your-feature`
4. Open a Pull Request describing your improvements or shared interview experiences

---

## License

This project is released under the [MIT License](LICENSE). Feel free to use, modify, and redistribute, but please retain the license notice.
