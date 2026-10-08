# Contact Angle Measurement (C++ / OpenCV)

A computer-vision tool that automatically measures the **contact angle** of a liquid droplet on a surface from a camera image. The contact angle shows how well a liquid wets a surface, and it is used in materials testing and quality control.

The algorithm is packaged as a C++ DLL, with an MFC demo application and a C# example that calls the same DLL.

## Features

- **Ellipse fitting** of the droplet outline using OpenCV
- **Baseline fitting** of the surface as a circle, so it works on **convex**, **concave** and **flat** surfaces
- Automatic detection of the left and right **contact points**
- Calculates **left and right contact angles**
- Handles images where a **dispensing needle** is still in the droplet
- Optional sub-pixel edge detection and manual baseline setting
- Returns all inputs and results as **JSON**

## Results

**Convex surface:** left 104.09°, right 108.08°

![Convex surface](screenshots/convex.png)

**Convex surface with needle in droplet:** left 111.77°, right 112.38°

![Convex surface with needle](screenshots/convexhaspin.png)

**Flat surface:** left 148.28°, right 147.92°

![Flat surface](screenshots/horizon.png)

**Concave surface:** left 76.41°, right 80.24°

![Concave surface](screenshots/concave.png)

Blue: fitted droplet ellipse. Red: fitted surface baseline. The short lines at the contact points show the tangents used to calculate each angle.

## Project structure

| Folder | Contents |
|---|---|
| `JiaoShuiEllipseFitDLL` | Core algorithm (C++ DLL) |
| `Demo` | MFC demo application |
| `TestCSharp` | C# example calling the DLL |
| `cvLib`, `inc`, `lib`, `third_party` | OpenCV and other dependencies |
| `doc` | Documentation |

## Tools used

- C++, OpenCV
- MFC (Windows desktop UI)
- C# / .NET with Newtonsoft.Json
- Visual Studio (x64)

## How to build

1. Open `Demo.sln` in Visual Studio.
2. Select the **x64** platform.
3. Build the solution and run the **Demo** project.

Note: the demo interface is in Chinese.
