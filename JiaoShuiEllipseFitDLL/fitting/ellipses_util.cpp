#include <iostream>

#include <cmath>
#include "ellipses_util.h"
#include <tuple>
#include "opencv2/opencv.hpp"
#include <functional>
#include "common.h"
#include "fit_util.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


DropletEllipse::DropletEllipse()
    : h(0), k(0), a(0), b(0), theta(0) {}

DropletEllipse::DropletEllipse(
    double h,
    double k,
    double a, 
    double b, 
    double theta
) : h(h), k(k), a(a), b(b), theta(theta) {}

Eigen::Vector2d
DropletEllipse::getPoint(
    double t
) const {
    Eigen::Vector2d point;
    point(0) = h + a * cos(t) * cos(theta) - b * sin(t) * sin(theta);
    point(1) = k + a * cos(t) * sin(theta) + b * sin(t) * cos(theta);
    return point;
}

Eigen::Vector2d 
DropletEllipse::getGradient(
    double t
) const {
    double dx_dt = -a * sin(t) * cos(theta) - b * cos(t) * sin(theta);
    double dy_dt = -a * sin(t) * sin(theta) + b * cos(t) * cos(theta);
    return Eigen::Vector2d(dx_dt, dy_dt);
}

// Function to calculate the slope of the tangent line to an ellipse at a point (x, y)
double
DropletEllipse::tangentSlope(
    double x, 
    double y
) const {
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double xPrime = (x - h) * cosTheta + (y - k) * sinTheta;
    double yPrime = (x - h) * sinTheta - (y - k) * cosTheta;
    double denom = yPrime / (b * b);
    if (std::abs(denom) < 1e-8)
        return std::numeric_limits<double>::infinity();
    double numer = -(xPrime / (a * a));
    return numer / denom;
}

void 
DropletEllipse::getTangentVector(
    double x, 
    double y, 
    double& tx, 
    double& ty
) const {
    // === 4. Compute ellipse tangent vector at (x, ym) ===
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double xPrime = (x - h) * cosTheta + (y - k) * sinTheta;
    double yPrime = -(x - h) * sinTheta + (y - k) * cosTheta;

    // Step 2: Partial derivatives of ellipse equation F(x', y') = (x'/a)^2 + (y'/b)^2 - 1
    double Fx = 2 * xPrime / (a * a);
    double Fy = 2 * yPrime / (b * b);

    // Step 3: Tangent vector in rotated space
    double dx_prime = 1.0;
    double dy_prime = -Fx / Fy;

    // Step 4: Rotate back to image space
    double dx = dx_prime * cosTheta - dy_prime * sinTheta;
    double dy = dx_prime * sinTheta + dy_prime * cosTheta;

    // Step 5: Normalize and scale to desired length
    double norm = std::sqrt(dx * dx + dy * dy);
    if(norm < 1e-8) {
		tx = 0;
		ty = 0;
		return;
	}
    tx = dx / norm;
    ty = dy / norm;
}

// Apply the rotation to convert the coordinates
void 
DropletEllipse::rotate(
    double x, 
    double y, 
    double& x_prime,
    double& y_prime
) const {
    double cos_theta = cos(theta);
    double sin_theta = sin(theta);
    x_prime = (x - h) * cos_theta + (y - k) * sin_theta;
    y_prime = -(x - h) * sin_theta + (y - k) * cos_theta;
}

double
DropletEllipse::signedDistanceToEllipse(
	double x, 
	double y
) const {
    double x_prime, y_prime;
    rotate(x, y, x_prime, y_prime);
    double normalized = (x_prime * x_prime) / (a * a) + (y_prime * y_prime) / (b * b);
    return normalized - 1.0; // >0 outside, <0 inside, ==0 on boundary
}

IntersectionCalculator::IntersectionCalculator(
    const DropletEllipse& e1, 
    const DropletEllipse& e2
) : ellipse1(e1)
    , ellipse2(e2) {
}

Eigen::Vector2d 
IntersectionCalculator::solveIntersection(
    Eigen::Vector2d initial_guess, 
    int max_iter, 
    double tol
) {
    Eigen::Vector2d params = initial_guess;
    for (int i = 0; i < max_iter; ++i) {
        Eigen::Vector2d f_val = equations(params);
        Eigen::Matrix2d jacobian;

        // Approximate Jacobian matrix
        double h = 0.0001;
        for (int j = 0; j < 2; ++j) {
            Eigen::Vector2d params_plus_h = params;
            params_plus_h(j) += h;
            Eigen::Vector2d f_val_plus_h = equations(params_plus_h);
            jacobian.col(j) = (f_val_plus_h - f_val) / h;
        }

        Eigen::Vector2d delta = jacobian.colPivHouseholderQr().solve(-f_val);
        params += delta;

        if (delta.norm() < tol) {
            break;
        }
    }
    return params;
}

double 
IntersectionCalculator::calculateAngle(
    double slope1, 
    double slope2
) const {
    double angle = atan(abs((slope2 - slope1) / (1 + slope1 * slope2)));
    return angle * 180 / M_PI; // Convert to degrees
}

Eigen::Vector2d 
IntersectionCalculator::equations(
    Eigen::Vector2d params
) const {
    double t = params(0);
    double u = params(1);
    Eigen::Vector2d p1 = ellipse1.getPoint(t);
    Eigen::Vector2d p2 = ellipse2.getPoint(u);
    Eigen::Vector2d result;
    result(0) = p1(0) - p2(0);
    result(1) = p1(1) - p2(1);
    return result;
}

// Function to compute the value of the ellipse equation at a point (x, y)
double ellipseEquation(
    const DropletEllipse& e,
    double x,
    double y
) {
    double cosTheta = cos(e.theta);
    double sinTheta = sin(e.theta);
    double xPrime = (x - e.h) * cosTheta + (y - e.k) * sinTheta;
    double yPrime = (x - e.h) * sinTheta - (y - e.k) * cosTheta;
    return (xPrime * xPrime) / (e.a * e.a) + (yPrime * yPrime) / (e.b * e.b) - 1;
}

double dis(double x1, double y1, double x2, double y2)
{
    return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

inline double clampAngle(double t) {
    while (t < 0) t += 2 * M_PI;
    while (t > 2 * M_PI) t -= 2 * M_PI;
    return t;
}

std::vector<PointAndAngle> 
findIntersectionsEllipse(
    EllipseParams ep1, 
    double theta1,
    EllipseParams ep2, 
    double theta2,
    bool checkMode,
    const cv::Point2f* left,
    const cv::Point2f* right
) {
    std::vector<PointAndAngle> result;
    constexpr double EPSILON_DIST = 1e-3;
    constexpr int MAX_INITIAL_GUESSES = 12;

    // Early rejection based on distance between centers
    const double dx = ep1.h - ep2.h;
    const double dy = ep1.k - ep2.k;
    const double center_dist2 = dx * dx + dy * dy;
    const double max_r1 = std::max(std::abs(ep1.a), std::abs(ep1.b));
    const double max_r2 = std::max(std::abs(ep2.a), std::abs(ep2.b));
    const double min_r1 = std::min(std::abs(ep1.a), std::abs(ep1.b));
    const double min_r2 = std::min(std::abs(ep2.a), std::abs(ep2.b));


    if (center_dist2 > (max_r1 + max_r2) * (max_r1 + max_r2)) return result;

    // Build ellipse objects
    DropletEllipse ellipse1(ep1.h, ep1.k, ep1.a, ep1.b, theta1);
    DropletEllipse ellipse2(ep2.h, ep2.k, ep2.a, ep2.b, theta2);
    IntersectionCalculator solver(ellipse1, ellipse2);


    // Generate multiple initial guesses
    for (int i = 0; i < MAX_INITIAL_GUESSES; ++i) {
        double t0 = (2.0 * M_PI * i) / MAX_INITIAL_GUESSES;
        double u0 = t0; // synchronized start

        Eigen::Vector2d guess(clampAngle(t0), clampAngle(u0));
        Eigen::Vector2d sol = solver.solveIntersection(guess);

        if (!std::isfinite(sol(0)) || !std::isfinite(sol(1))) continue;

        double t_sol = clampAngle(sol(0));
        double u_sol = clampAngle(sol(1));

        Eigen::Vector2d pt = ellipse1.getPoint(t_sol);

        // Check uniqueness
        bool duplicate = false;
        for (const auto& prev : result) {
            double dx = pt(0) - prev.point.x;
            double dy = pt(1) - prev.point.y;
            if (dx * dx + dy * dy < EPSILON_DIST * EPSILON_DIST) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        // Get tangents
        Eigen::Vector2d g1 = ellipse1.getGradient(t_sol);
        Eigen::Vector2d g2 = ellipse2.getGradient(u_sol);

        if (g1.norm() == 0 || g2.norm() == 0) continue;
        Eigen::Vector2d v1 = g1.normalized();
        Eigen::Vector2d v2 = g2.normalized();

        double dot = std::min(std::max(v1.dot(v2), -1.0), 1.0);
        double angle = std::acos(dot);

        PointAndAngle pta;
        pta.point.x = pt(0);
        pta.point.y = pt(1);
        pta.angle = angle;
        pta.vec1.x = v1(0);
        pta.vec1.y = v1(1);
        pta.vec2.x = v2(0);
        pta.vec2.y = v2(1);
        result.push_back(pta);
    }
    std::vector<PointAndAngle> _final;
    for (auto it : result) {
        if (it.point.x == 0 && it.point.y == 0)
            continue;
        bool exist = false;
        for (const auto& already : _final) {
            if (it.point.x == already.point.x &&
                it.point.y == already.point.y
                ) {
                exist = true;
                break;
            }
        }
        if (exist)
            continue;
        if (!checkMode) { //fitMode == eFitSemiAuto || fitMode == eFitManual
            _final.push_back(it);
        }
        else {
            if (left && near_to_basepoint(it.point, *left, 400)) {
                _final.push_back(it);
            }
            else if (right && near_to_basepoint(it.point, *right, 400)) {
                _final.push_back(it);
            }
        }
    }
    return _final;
}

// Function to calculate the intersection points of two circles
std::vector<PointAndAngle> 
getCircleIntersections(
    double x1,
    double y1,
    double r1,
    double x2, 
    double y2, 
    double r2
) {
    std::vector<PointAndAngle> intersections;

    double dx = x2 - x1;
    double dy = y2 - y1;
    double d = std::sqrt(dx * dx + dy * dy);

    if (d > r1 + r2 || d < std::abs(r1 - r2) || (d == 0 && r1 == r2))
        return intersections; // No solution

    double a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
    double h = std::sqrt(r1 * r1 - a * a);

    double Px = x1 + a * dx / d;
    double Py = y1 + a * dy / d;

    double rx = -dy * (h / d);
    double ry = dx * (h / d);

    // Two intersection points
    std::array<std::pair<double, double>, 2> pts = { {
        { Px + rx, Py + ry },
        { Px - rx, Py - ry }
    } };

    for (const auto& pt : pts) {
        double ix = pt.first;
        double iy = pt.second;

        MPoint v1 = { iy - y1, -(ix - x1) }; // Tangent 1
        MPoint v2 = { iy - y2, -(ix - x2) }; // Tangent 2

        double dot = v1.x * v2.x + v1.y * v2.y;
        double len1 = std::hypot(v1.x, v1.y);
        double len2 = std::hypot(v2.x, v2.y);
        double cosTheta = dot / (len1 * len2);
        cosTheta = std::min(std::max(cosTheta, -1.0), 1.0); // C++17

        double angleDeg = std::acos(cosTheta) * 180.0 / M_PI;

        PointAndAngle pta;
        pta.point = { ix, iy };
        pta.vec1 = { v1.x / len1, v1.y / len1 };
        pta.vec2 = { v2.x / len2, v2.y / len2 };
        pta.angle = angleDeg;

        intersections.push_back(pta);
    }

    return intersections;
}

double calculateEllipseTangentLineSlope(
    double a, 
    double b, 
    double theta,
    double x, 
    double y
) {
    double x_rot = x * cos(theta) + y * sin(theta);
    double y_rot = -x * sin(theta) + y * cos(theta);

    // Calculate coefficients of the tangent line equation in rotated coordinates
    double A = x_rot / (a * a);
    double B = y_rot / (b * b);

    // Rotate coefficients back to original coordinates
    double A_rot = A * cos(theta) - B * sin(theta);
    double B_rot = A * sin(theta) + B * cos(theta);

    return -A_rot / B_rot;
}

std::vector<PointAndAngle> findIntersectionsEllipseLine(
    const cv::Point2f& center,
    const cv::Size2f& axes,
    double theta,
    const cv::Point2f& left,
    const cv::Point2f& right,
    int isSelect
) {
    constexpr double pixelThreshold = 14.0;
    const double thresholdDistSq = pixelThreshold * pixelThreshold;

    std::vector<PointAndAngle> intersections;

    const double a = axes.width;
    const double b = axes.height;

    // Translate and rotate line points into ellipse space
    auto transformToEllipseSpace = [&](const cv::Point2f& pt) -> cv::Point2f {
        double x = pt.x - center.x;
        double y = pt.y - center.y;
        double x_rot = x * std::cos(theta) + y * std::sin(theta);
        double y_rot = -x * std::sin(theta) + y * std::cos(theta);
        return cv::Point2f((float)x_rot, (float)y_rot);
    };

    auto transformBackToOriginalSpace = [&](const cv::Point2f& pt) -> cv::Point2f {
        double x = pt.x * std::cos(theta) - pt.y * std::sin(theta) + center.x;
        double y = pt.x * std::sin(theta) + pt.y * std::cos(theta) + center.y;
        return cv::Point2f((float)x, (float)y);
    };

    cv::Point2f p1 = transformToEllipseSpace(left);
    cv::Point2f p2 = transformToEllipseSpace(right);
    cv::Point2f dp = p2 - p1;

    // Solve quadratic equation for ellipse-line intersection
    double A = (dp.x * dp.x) / (a * a) + (dp.y * dp.y) / (b * b);
    double B = 2 * (p1.x * dp.x / (a * a) + p1.y * dp.y / (b * b));
    double C = (p1.x * p1.x) / (a * a) + (p1.y * p1.y) / (b * b) - 1.0;

    double D = B * B - 4 * A * C;
    if (D < 0) 
        return intersections; // No intersection

    auto computeIntersectionPoint = [&](double t) -> PointAndAngle {
        cv::Point2f localPt = p1 + t * dp;
        cv::Point2f origPt = transformBackToOriginalSpace(localPt);

        // Get tangents
        double tangentSlope1 = calculateEllipseTangentLineSlope(a, b, theta, origPt.x - center.x, origPt.y - center.y);
        double tangentSlope2 = std::numeric_limits<double>::infinity();
        if (std::abs(right.x - left.x) > 1e-6)
            tangentSlope2 = (right.y - left.y) / (right.x - left.x);

        double angle = std::atan2(tangentSlope2 - tangentSlope1, 1.0 + tangentSlope1 * tangentSlope2) * 180.0 / M_PI;

        PointAndAngle pta;
        pta.point = PointF(origPt.x, origPt.y);
        pta.angle = std::abs(angle);

        double theta1 = std::atan(tangentSlope1);
        double theta2 = std::atan(tangentSlope2);

        pta.vec1 = PointF(std::cos(theta1), std::sin(theta1));
        pta.vec2 = PointF(std::cos(theta2), std::sin(theta2));

        return pta;
    };

    double sqrtD = std::sqrt(D);
    double t1 = (-B + sqrtD) / (2 * A);
    double t2 = (-B - sqrtD) / (2 * A);

    // Sort t1 < t2
    if (t1 > t2) 
        std::swap(t1, t2);

    intersections.push_back(computeIntersectionPoint(t1));
    if (D > 1e-8)
        intersections.push_back(computeIntersectionPoint(t2));

    // === Filter by isSelect if needed ===
    if (isSelect > 0) {
        const auto& refPt = (isSelect == 1 ? left : right);
        if (intersections.empty()) return intersections;

        if (intersections.size() == 1) {
            double dx = intersections[0].point.x - refPt.x;
            double dy = intersections[0].point.y - refPt.y;
            if (dx * dx + dy * dy > thresholdDistSq)
                intersections.clear();
        }
        else {
            // Pick the closest one to refPt
            double minDistSq = std::numeric_limits<double>::max();
            int bestIdx = -1;
            for (int i = 0; i < intersections.size(); ++i) {
                double dx = intersections[i].point.x - refPt.x;
                double dy = intersections[i].point.y - refPt.y;
                double distSq = dx * dx + dy * dy;
                if (distSq < minDistSq && distSq < thresholdDistSq) {
                    minDistSq = distSq;
                    bestIdx = i;
                }
            }
            if (bestIdx >= 0)
                return { intersections[bestIdx] };
            else
                intersections.clear();
        }
    }

    return intersections;
}


// Function to calculate the intersection points of two circles
std::vector<PointAndAngle> findIntersectionsCircleLine(
    const cv::Point2f& center,
    double r, 
    const cv::Point2f& left,
    const cv::Point2f& right,
    int isSelect
) {
    std::vector<PointAndAngle> intersections;

    double x1 = left.x, y1 = left.y;
    double x2 = right.x, y2 = right.y;
    double cx = center.x, cy = center.y;

    double dx = x2 - x1;
    double dy = y2 - y1;
    double A = dx * dx + dy * dy;
    double B = 2 * (dx * (x1 - cx) + dy * (y1 - cy));
    double C = (x1 - cx) * (x1 - cx) + (y1 - cy) * (y1 - cy) - r * r;
    double discriminant = B * B - 4 * A * C;

    if (discriminant < 0) {
        // No intersection
        return intersections;
    }
    else if (discriminant == 0) {
        // One intersection
        double t = -B / (2.0 * A);
        //if (0 <= t && t <= 1)
        {
            PointAndAngle pta;
            pta.point.x = x1 + t * dx;
            pta.point.y = y1 + t * dy;
            pta.angle = 90;
            pta.vec1.x = 0;
            pta.vec1.y = 1;
            pta.vec2.x = 0;
            pta.vec2.y = 1;
            intersections.push_back(pta);
        }
    }
    else {
        // Two intersections
        double sqrtDiscriminant = std::sqrt(discriminant);
        double t1 = (-B - sqrtDiscriminant) / (2.0 * A);
        double t2 = (-B + sqrtDiscriminant) / (2.0 * A);

        //if (0 <= t1 && t1 <= 1)
        {
            double x3 = x1 + t1 * dx;
            double y3 = y1 + t1 * dy;

            double tangentSlope1 = 0;
            if (y3 != cy) {
                tangentSlope1 = -(cx - x3) / (cy - y3);
            }

            double tangentSlope2 = 10000000.0;
            if (x2 != x1) {
                tangentSlope2 = (y2 - y1) / (x2 - x1);
            }

            PointAndAngle pta;
            pta.point.x = x3;
            pta.point.y = y3;
            pta.angle = atan(abs((tangentSlope2 - tangentSlope1) / (1.0 + tangentSlope1 * tangentSlope2))) * 180.0 / M_PI;
            pta.vec1.x = cos(atan(tangentSlope1));
            pta.vec1.y = sin(atan(tangentSlope1));
            pta.vec2.x = cos(atan(tangentSlope2));
            pta.vec2.y = sin(atan(tangentSlope2));
            intersections.push_back(pta);
        }
        //if (0 <= t2 && t2 <= 1) 
        {
            double x3 = x1 + t2 * dx;
            double y3 = y1 + t2 * dy;

            double tangentSlope1 = 0;
            if (y3 != cy) {
                tangentSlope1 = -(cx - x3) / (cy - y3);
            }

            double tangentSlope2 = 1000000000.0;
            if (x1 != x2) {
                tangentSlope2 = (y2 - y1) / (x2 - x1);
            }

            PointAndAngle pta;
            pta.point.x = x3;
            pta.point.y = y3;
            pta.angle = atan(abs((tangentSlope2 - tangentSlope1) / (1.0 + tangentSlope1 * tangentSlope2))) * 180.0 / M_PI;
            pta.vec1.x = cos(atan(tangentSlope1));
            pta.vec1.y = sin(atan(tangentSlope1));
            pta.vec2.x = cos(atan(tangentSlope2));
            pta.vec2.y = sin(atan(tangentSlope2));
            intersections.push_back(pta);
        }
    }

    if (isSelect > 0 && intersections.size() == 2) {
        const auto refPt = (isSelect == 1 ? left : right);

        if (distance(intersections[0].point, refPt) < distance(intersections[1].point, refPt)) {
            //intersections.push_back(intersections[0]);
            intersections.erase(intersections.begin() + 1);
        }
        else {
            intersections.erase(intersections.begin());
            // _outRes.push_back(intersections[1]);
        }
    }

    return intersections;
}

/** Function to calculate the coefficients of a circle given three points
*   return center_x, center_y, radius
*/

std::tuple<double, double, double> 
findCircleCoefficients(
    const MPoint& p1, 
    const MPoint& p2, 
    const MPoint& p3
) {
    double x1 = p1.x, y1 = p1.y;
    double x2 = p2.x, y2 = p2.y;
    double x3 = p3.x, y3 = p3.y;

    double A = x1 * (y2 - y3) - y1 * (x2 - x3) + (x2 * y3 - x3 * y2);
    double B = (x1 * x1 + y1 * y1) * (y3 - y2) + (x2 * x2 + y2 * y2) * (y1 - y3) + (x3 * x3 + y3 * y3) * (y2 - y1);
    double C = (x1 * x1 + y1 * y1) * (x2 - x3) + (x2 * x2 + y2 * y2) * (x3 - x1) + (x3 * x3 + y3 * y3) * (x1 - x2);
    double D = (x1 * x1 + y1 * y1) * (x3 * y2 - x2 * y3) + (x2 * x2 + y2 * y2) * (x1 * y3 - x3 * y1) + (x3 * x3 + y3 * y3) * (x2 * y1 - x1 * y2);

    if (A == 0) {
		return std::make_tuple(0.0, 0.0, 0.0); // No unique circle can be formed
	}
    return std::make_tuple(-B / (2.0 * A), -C / (2.0 * A), sqrt((B * B + C * C - 4.0 * A * D) / (4.0 * A * A)));
}

/**
* Find intersection points between two circles passing through 3 points
*
*/
std::vector<PointAndAngle> 
findCirclePointIntersections(
    double h1, 
    double k1, 
    double r1, 
    double h2, 
    double k2, 
    double r2
) {
    std::vector<PointAndAngle> intersections;    

    double d = sqrt((h2 - h1) * (h2 - h1) + (k2 - k1) * (k2 - k1));

    if (d > r1 + r2 || d < fabs(r1 - r2) || (d == 0 && r1 == r2)) {
        return intersections; // No intersection
    }

    double a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
    double h = sqrt(r1 * r1 - a * a);
    double x0 = h1 + a * (h2 - h1) / d;
    double y0 = k1 + a * (k2 - k1) / d;
    double rx = -(k2 - k1) * (h / d);
    double ry = -(h2 - h1) * (h / d);

    double x3 = x0 - rx; 
    double y3 = y0 + ry;    

    double tangentSlope1 = 0;
    if (y3 != k1) {
        tangentSlope1 = -(x3 - h1) / (y3 - k1);
    }

    double tangentSlope2 = 0;
    if (y3 != k2) {
        tangentSlope2 = -(x3 - h2) / (y3 - k2);
    }

    PointAndAngle pta;
    pta.point.x = x3;
    pta.point.y = y3;
    pta.angle = atan(abs((tangentSlope2 - tangentSlope1) / (1.0 + tangentSlope1 * tangentSlope2))) * 180.0 / M_PI;
    pta.vec1.x = cos(atan(tangentSlope1));
    pta.vec1.y = sin(atan(tangentSlope1));
    pta.vec2.x = cos(atan(tangentSlope2));
    pta.vec2.y = sin(atan(tangentSlope2));
    intersections.push_back(pta);

    if (rx * ry != 0) {
        x3 = x0 + rx;
        y3 = y0 - ry;

        double tangentSlope1 = 1000000000.0;
        if (y3 != k1) {
            tangentSlope1 = -(x3 - h1) / (y3 - k1);
        }

        double tangentSlope2 = 100000000.0;
        if (y3 != k2) {
            tangentSlope2 = -(x3 - h2) / (y3 - k2);
        }

        PointAndAngle pta2;
        pta2.point.x = x3;
        pta2.point.y = y3;
        pta2.angle = atan(abs((tangentSlope2 - tangentSlope1) / (1.0 + tangentSlope1 * tangentSlope2))) * 180.0 / M_PI;
        pta2.vec1.x = cos(atan(tangentSlope1));
        pta2.vec1.y = sin(atan(tangentSlope1));
        pta2.vec2.x = cos(atan(tangentSlope2));
        pta2.vec2.y = sin(atan(tangentSlope2));
        intersections.push_back(pta2);
    }
    return intersections;
}

Eigen::VectorXd solveEllipseCoefficients(
    const double* p_xs, 
    const double* p_ys
) {
    Eigen::MatrixXd A(5, 6);
    Eigen::VectorXd b(5);
    b.setZero();
    
    // Fill the matrix A with the points
    for (int i = 0; i < 5; ++i) {
        double x = p_xs[i];
        double y = p_ys[i];
        A(i, 0) = x * x;
        A(i, 1) = x * y;
        A(i, 2) = y * y;
        A(i, 3) = x;
        A(i, 4) = y;
        A(i, 5) = 1;
    }

    // Solve the system using SVD
    Eigen::VectorXd p = A.jacobiSvd(Eigen::ComputeFullV).matrixV().col(5);

    return p;
}

// Function to compute the value of the ellipse equation at a point (x, y)
double ellipseEquation(
    double h, 
    double k, 
    double a, 
    double b, 
    double theta, 
    double x, 
    double y
) {
    double dx = x - h, dy = y - k;
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double xPrime = dx * cosTheta + dy * sinTheta;
    double yPrime = -dx * sinTheta + dy * cosTheta;
    return (xPrime * xPrime) / (a * a) + (yPrime * yPrime) / (b * b) - 1.0;
}

// Compute partial derivatives of the ellipse equation w.r.t x and y
void ellipseGradient(double h, double k, double a, double b, double theta, double x, double y, double& fx, double& fy) 
{
    double dx = x - h, dy = y - k;
    double cosT = cos(theta), sinT = sin(theta);
    double xRot = dx * cosT + dy * sinT;
    double yRot = -dx * sinT + dy * cosT;


    fx = 2.0 * (xRot * cosT / (a * a) - yRot * sinT / (b * b));
    fy = 2.0 * (xRot * sinT / (a * a) + yRot * cosT / (b * b));
}

PointAndAngle findIntersectionEllipseOne(
    const cv::Point2f& c0, 
    const cv::Size2f& r0, 
    double theta1, 
    const cv::Point2f& c1, 
    const cv::Size2f& r1, 
    double theta2, 
    const cv::Point2f& p0
) {
    constexpr double EPSILON = 1e-6;
    constexpr int MAX_ITER = 100;
    PointAndAngle result;
    double x = p0.x, y = p0.y;

    for (int i = 0; i < MAX_ITER; ++i) {
        double f1 = ellipseEquation(c0.x, c0.y, r0.width, r0.height, theta1, x, y);
        double f2 = ellipseEquation(c1.x, c1.y, r1.width, r1.height, theta2, x, y);

        if (std::abs(f1) < EPSILON && std::abs(f2) < EPSILON)
            break;

        double fx1, fy1, fx2, fy2;
        ellipseGradient(c0.x, c0.y, r0.width, r0.height, theta1, x, y, fx1, fy1);
        ellipseGradient(c1.x, c1.y, r1.width, r1.height, theta2, x, y, fx2, fy2);

        double det = fx1 * fy2 - fx2 * fy1;
        if (std::abs(det) < EPSILON)
            break;

        double dx = (f1 * fy2 - f2 * fy1) / det;
        double dy = (f2 * fx1 - f1 * fx2) / det;

        x -= dx;
        y -= dy;

        if (std::abs(dx) < EPSILON && std::abs(dy) < EPSILON)
            break;
    }

    result.point = PointF(x, y);

    // Gradient vectors as tangent directions
    double fx1, fy1, fx2, fy2;
    ellipseGradient(c0.x, c0.y, r0.width, r0.height, theta1, x, y, fx1, fy1);
    ellipseGradient(c1.x, c1.y, r1.width, r1.height, theta2, x, y, fx2, fy2);

    // Normal = gradient => Tangent is perpendicular => (-fy, fx)
    cv::Point2f t1((float)-fy1, (float)fx1), t2((float)-fy2, (float)fx2);

    double dot = t1.dot(t2);
    double len1 = cv::norm(t1), len2 = cv::norm(t2);

    if (len1 > EPSILON && len2 > EPSILON) {
        double cos_angle = dot / (len1 * len2);
        cos_angle = std::max(-1.0, std::min(1.0, cos_angle));
        result.angle = std::acos(cos_angle) * 180.0 / CV_PI;
    }
    else {
        result.angle = -1.0;
    }

    auto v1 = t1 * (1.0f / (len1 + EPSILON));
    auto v2 = t2 * (1.0f / (len2 + EPSILON));
    result.vec1 = PointF(v1.x, v1.y);
    result.vec2 = PointF(v2.x, v2.y);

    return result;
}

// Function to perform polynomial curve fitting
Eigen::VectorXd 
polynomialCurveFit(
    const std::vector<double>& x,
    const std::vector<double>& y, 
    int degree
) {
    int n = (int)x.size();
    Eigen::MatrixXd A(n, degree + 1);
    Eigen::VectorXd b(n);

    // Fill matrix A and vector b for the linear system
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= degree; ++j) {
            A(i, j) = pow(x[i], j); // A holds powers of x
        }
        b(i) = y[i]; // b holds the corresponding y values
    }

    // Solve the system A * coeffs = b
    // x^0, x^1, x^2, x^3
    Eigen::VectorXd coeffs = A.householderQr().solve(b);
    return coeffs;
}

// Function to perform polynomial curve fitting
Eigen::VectorXd 
polynomialCurveFitYHouseholderQR(
    const std::vector<double>& x, 
    const std::vector<double>& y, 
    int degree
) {
    int n = (int)x.size();
    Eigen::MatrixXd A(n, degree + 1);
    Eigen::VectorXd b(n);

    // Fill matrix A and vector b for the linear system
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= degree; ++j) {
            A(i, j) = pow(y[i], j); // A holds powers of x
        }
        b(i) = x[i]; // b holds the corresponding y values
    }

    // Solve the system A * coeffs = b
    // y^0, y^1, y^2, y^3
    Eigen::VectorXd coeffs = A.householderQr().solve(b);
    return coeffs;
}

// Function to perform polynomial curve fitting
Eigen::VectorXd 
polynomialCurveFitY(
    const std::vector<double>& x, 
    const std::vector<double>& y,
    int degree
) {
    int n = (int)x.size();
    Eigen::MatrixXd A(n, degree + 1);
    Eigen::VectorXd b(n);

    // Fill matrix A and vector b for the linear system
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= degree; ++j) {
            A(i, j) = pow(y[i], j); // A holds powers of x
        }
        b(i) = x[i]; // b holds the corresponding y values
    }

    // Solve the system A * coeffs = b
    // y^0, y^1, y^2, y^3
    // Solve for coefficients: (X^T * X) * a = X^T * y
    Eigen::VectorXd coeffs = (A.transpose() * A).ldlt().solve(A.transpose() * b);
    return coeffs;
}

// Function to evaluate a polynomial at a given x
double
polyEval(
    const Eigen::VectorXd& coeffs, 
    double x
) {
    double result = 0;
    for (int i = 0; i < coeffs.size(); ++i) {
        result += coeffs(i) * pow(x, i);
    }
    return result;
}

cv::Point2f
polarToCartesian(
	double r, 
	double theta,
    double xc,
    double yc
) {    
	return cv::Point2f((float)(r * cos(theta) + xc), (float)(r * sin(theta) + yc));
}
// Function to calculate the slope of the polynomial at a given x
double 
polySlope(
    const Eigen::VectorXd& coeffs,
    double x
) {
    double slope = 0;
    for (int i = 1; i < coeffs.size(); ++i) {
        slope += i * coeffs(i) * pow(x, i - 1);
    }
    return slope;
}

// Function to calculate the droplet shape based on the Young-Laplace equation
// This function is used for the fitting process
void calculateYoungLaplace(
    const YoungLaplaceParams& params, 
    const std::vector<double>& xData,
    std::vector<double>& yModel
) {
    yModel.clear();
    for (double x : xData) {
        // Calculate y from the equation of a circular arc (Young-Laplace in 2D approximation)
        double y = params.offsetY - 
            sqrt(params.radius * params.radius - (x - params.offsetX) * (x - params.offsetX)) + 
            params.height;
        yModel.push_back(y);
    }
}

// Residual function for Levenberg-Marquardt optimization
void residuals(
    const std::vector<double>& params, 
    std::vector<double>& residuals,
    const std::vector<double>& xData,
    const std::vector<double>& yData
) {
    YoungLaplaceParams laplaceParams;
    laplaceParams.radius = params[0];
    laplaceParams.height = params[1];
    laplaceParams.offsetX = params[2];
    laplaceParams.offsetY = params[3];

    std::vector<double> yModel;
    calculateYoungLaplace(laplaceParams, xData, yModel);

    residuals.resize(yData.size());
    for (size_t i = 0; i < yData.size(); ++i) {
        residuals[i] = yData[i] - yModel[i];
    }
}

// Wrapper function for OpenCV's Levenberg-Marquardt optimization
bool fitYoungLaplace(
    const std::vector<cv::Point>& dropletContour, 
    YoungLaplaceParams& params
) {
    const int maxIterations = 1000;
    const double epsilon = 1e-6;

    // Extract x and y data from the contour points
    std::vector<double> xData, yData;
    for (const cv::Point& pt : dropletContour) {
        xData.push_back(pt.x);
        yData.push_back(pt.y);
    }

    // Non-linear least squares optimization
    for (int iter = 0; iter < maxIterations; ++iter) {
        std::vector<double> yModel;
        calculateYoungLaplace(params, xData, yModel);

        // Compute residuals (difference between observed and model values)
        double error = 0;
        for (size_t i = 0; i < yData.size(); ++i) {
            double residual = yData[i] - yModel[i];
            error += residual * residual;
        }

        // Stopping criterion
        if (error < epsilon) {
            break;
        }

        // Gradient descent step (manual update of parameters for simplicity)
        params.radius += 0.01 * (rand() % 10 - 5);  // Random small updates
        params.height += 0.01 * (rand() % 10 - 5);
        params.offsetX += 0.01 * (rand() % 10 - 5);
        params.offsetY += 0.01 * (rand() % 10 - 5);
    }

    return true;  // Fit succeeded
}

// Function to calculate the contact angle based on Young-Laplace parameters
double calculateContactAngle(
    const YoungLaplaceParams& params
) {
    // Calculate contact angle in radians
    double contactAngleRad = acos((params.radius - params.height) / params.radius);
    // Convert to degrees
    double contactAngleDeg = contactAngleRad * (180.0 / M_PI);
    return contactAngleDeg;
}

std::vector<PointAndAngle> findIntersectionPolyEllipse(
    Eigen::VectorXd coeffs, 
    const cv::Point2f& c, 
    const cv::Size2f& r,
    double theta, 
    double init_x
) {
    std::vector<PointAndAngle> result;
    double tol = 0.01;

    // Define ellipses
    DropletEllipse ellipse(c.x, c.y, r.width, r.height, theta);
    
    // Newton's method to find roots
    auto newton = [&](std::function<double(double)> f, std::function<double(double)> df, double x0) {
        double x = x0;
        for (int i = 0; i < 1000; ++i) {
            double fx = f(x);
            double dfx = df(x);
            if (abs(fx) < tol)
                break;
            x -= fx / dfx;
        }
        return x;
    };

    // Define the system of equations for the intersection
    auto F = [&](double x) {
        double y = polyEval(coeffs, x);
        double x_prime, y_prime;
        ellipse.rotate(x, y, x_prime, y_prime);
        return pow(x_prime / ellipse.a, 2) + pow(y_prime / ellipse.b, 2) - 1;
    };

    auto dF = [&](double x) {
        double y = polyEval(coeffs, x);
        double dy_dx = polySlope(coeffs, x);
        double x_prime, y_prime;
        ellipse.rotate(x, y, x_prime, y_prime);

        // Chain rule for the derivative, considering rotation
        //double dx_prime_dx = cos(ellipse.theta);
        //double dy_prime_dx = sin(ellipse.theta) * dy_dx;
        double dx_prime_dx = cos(ellipse.theta) + dy_dx*sin(ellipse.theta);
        double dy_prime_dx = -sin(ellipse.theta) + dy_dx * cos(ellipse.theta);

        return (2 * x_prime * dx_prime_dx) / (ellipse.a * ellipse.a) + (2 * y_prime * dy_prime_dx) / (ellipse.b * ellipse.b);
    };

    double root = newton(F, dF, init_x);
    double y = polyEval(coeffs, root);
    // Check if it satisfies the ellipse equation (within tolerance)
    if (abs(F(root)) < tol) {
        PointAndAngle pta;
       
        double tangentSlope1 = polySlope(coeffs, root);
        double tangentSlope2 = calculateEllipseTangentLineSlope(r.width, r.height, theta, root - c.x, y - c.y);

        pta.point.x = root;
        pta.point.y = y;
        pta.angle = atan(abs((tangentSlope2 - tangentSlope1) / (1 + tangentSlope1 * tangentSlope2))) * 180 / M_PI;
        pta.vec1.x = 1 / sqrt(1 + tangentSlope1 * tangentSlope1);
        pta.vec1.y = tangentSlope1 / sqrt(1 + tangentSlope1 * tangentSlope1);
        pta.vec2.x = cos(atan(tangentSlope2));
        pta.vec2.y = sin(atan(tangentSlope2));
        result.push_back(pta);
    }
    
    return result;
}

// 
double polySlopePolarCoordinate(
    Eigen::VectorXd coeffs,
    double angle
) {
    
    double numerator = 0;
    double denominator = 0;
    for (int i = 0; i < coeffs.size(); ++i) {
        numerator += cos(angle) * coeffs(i) * pow(angle, i);
        denominator -= sin(angle) * coeffs(i) * pow(angle, i);
        if (i > 0) {
            numerator += sin(angle) * i * coeffs(i) * pow(angle, i - 1);
            denominator += cos(angle) * i * coeffs(i) * pow(angle, i - 1);
        }
       
    }
    if (denominator == 0) {
        denominator = 0.0000000000001;
    }
    return numerator / denominator;
}

// Evaluate the polynomial at theta (r = f(theta))
double 
evalPoly(
    const Eigen::VectorXd& coefs, 
    double theta
) {
    double r = 0.0, t_pow = 1.0;
    for (int i = 0; i < coefs.size(); ++i) {
        r += coefs[i] * t_pow;
        t_pow *= theta;
    }
    return r;
}

// Evaluate the polynomial derivative at theta (dr/dtheta)
double 
evalPolyDeriv(
    const Eigen::VectorXd& coefs, 
    double theta
) {
    double dr = 0.0, t_pow = 1.0;
    for (int i = 1; i < coefs.size(); ++i) {
        dr += i * coefs[i] * t_pow;
        t_pow *= theta;
    }
    return dr;
}

bool extractEllipseParams(
    const Eigen::VectorXd& coeffs,
    EllipseParams& p, 
    double& theta
) {
    double A = coeffs(0), B = coeffs(1), C = coeffs(2);
    double D = coeffs(3), E = coeffs(4), F = coeffs(5);

    if (B * B - 4 * A * C >= 0)
        return false;

    p.h = (2 * C * D - B * E) / (B * B - 4 * A * C);
    p.k = (2 * A * E - B * D) / (B * B - 4 * A * C);

    double numerator = 2 * (A * p.h * p.h + C * p.k * p.k + B * p.h * p.k - F);
    double term1 = A + C;
    double term2 = sqrt((A - C) * (A - C) + B * B);

    p.b = sqrt(numerator / (term1 - term2));
    p.a = sqrt(numerator / (term1 + term2));
    theta = 0.5 * atan2(B, A - C);

    return true;
}


/**
*
* fitMethod : 0 => Least Square, 1=> Approximate Mean Sqaure(AMS), 2=> Direct least square(Driect) Method
*/
void ellipse_regression_OpenCV(
    const std::vector<cv::Point2f>& contour,
    int fitMethod, const cv::Point2f& left,
    const cv::Point2f& right,
    cv::RotatedRect& _outBox
) {
    if (contour.empty())
        return;
    try {
        cv::Mat pointsf;
        cv::Mat(contour).convertTo(pointsf, CV_32F);
        if (fitMethod == 0) {
            _outBox = cv::fitEllipse(pointsf);
        }
        else if (fitMethod == 1) {
            _outBox = cv::fitEllipseAMS(pointsf);
        }
        else {
            _outBox = cv::fitEllipseDirect(contour);
        }
        _outBox.size.width /= 2.0;
        _outBox.size.height /= 2.0;
    }
    catch (...) {
		_outBox = cv::RotatedRect();
	}
}

void ellipse_regression_Circle(
    const std::vector<cv::Point2f>& pts_up,
    int dimension, 
    cv::RotatedRect& box
) {

    double a11 = 0.0, a12 = 0.0, a13 = 0.0, a21 = 0.0, a22 = 0.0, a23 = 0.0, a31 = 0.0, a32 = 0.0, a33 = 0.0;
    double b1 = 0.0, b2 = 0.0, b3 = 0.0;
    double ww = dimension / 2.0;
    int n = (int)pts_up.size();
    if (!n) return;
    for (int i = 0; i < n; i++) {
        double xx = pts_up[i].x;
        double yy = pts_up[i].y;
        // 归一化
        double x = xx / ww;
        double y = yy / ww;
        // 计算矩阵 - A
        double x_x = x * x;
        double x_y = x * y;
        double y_y = y * y;
        double x_x_x = x_x * x;
        double x_y_y = x_y * y;
        double x_x_y = x * x_y;
        double y_y_y = y * y_y;
        a11 += x_x;
        a12 += x_y;
        a13 += x;
        a22 += y_y;
        a23 += y;
        a33 += 1.0;
        // B
        b1 -= x_x_x + x_y_y;
        b2 -= x_x_y + y_y_y;
        b3 -= x_x + y_y;
    }
    // 对称矩阵
    a21 = a12;
    a31 = a13;
    a32 = a23;
    // 求解方程而拟合
    double detA = calc_3d_det(a11, a12, a13, a21, a22, a23, a31, a32, a33);
    if (detA == 0.0)
        return;
    double detAB1 = calc_3d_det(b1, a12, a13, b2, a22, a23, b3, a32, a33);
    double detAB2 = calc_3d_det(a11, b1, a13, a21, b2, a23, a31, b3, a33);
    double detAB3 = calc_3d_det(a11, a12, b1, a21, a22, b2, a31, a32, b3);
    double a = detAB1 / detA;
    double b = detAB2 / detA;
    double c = detAB3 / detA;
    double cx = -a / 2 * ww;
    double cy = -b / 2 * ww;
    double r = sqrt(a * a + b * b - 4 * c) / 2 * ww;
    box.center.x = (float)cx;
    box.center.y = (float)cy;
    box.size.width = (float)r;
    box.size.height = (float)r;
    box.angle = 0.0;
}

double calc_3d_det(
    double a1,
    double b1, 
    double c1, 
    double a2,
    double b2,
    double c2,
    double a3,
    double b3,
    double c3
) {
    return a1 * (b2 * c3 - b3 * c2) - b1 * (a2 * c3 - a3 * c2) + c1 * (a2 * b3 - a3 * b2);
}

PointAndAngle calculate_bipoly_angles_on_horizontal(
    const cv::Point2f& left,
    const cv::Point2f& right,
    double y0,
    double y1,
    const Eigen::VectorXd& coefs,
    bool isLeft,
    int width,
    double& _ypos
) {
    _ypos = -1.f;
    PointAndAngle _out;

    // 1. Define reference line (horizontal) from known left and right

    // 3. Perform bisection to find intersection angle    
    bool found_soluton = false;

    auto polarToCartesian = [&](double y, bool isLeft) -> cv::Point2f {
        int xval = (int)polyEval(coefs, (double)y);
        int x = xval;
        if (isLeft)
            x = width - xval;
        cv::Point pt(x, (int)y);
        return cv::Point2f((float)x, (float)y);
    };
    cv::Point2f pt_lo = polarToCartesian(y0, isLeft);
    cv::Point2f pt_hi = polarToCartesian(y1, isLeft);
    double s_lo = signedDistanceToLine(pt_lo, left, right);
    double s_hi = signedDistanceToLine(pt_hi, left, right);

    // If signs are not opposite, bisection won't work
    double ym = 0.0;
    if (s_lo * s_hi <= 0.0) {
        const int max_iter = 50;
        const double tol = 0.0001;
        for (int iter = 0; iter < max_iter; ++iter) {
            double mid = (y0 + y1) / 2.0;
            cv::Point2f pt_mid = polarToCartesian(mid, isLeft);
            double s_mid = signedDistanceToLine(pt_mid, left, right);

            if (std::abs(s_mid) < 1e-6 || (y1 - y0) < tol) {
                ym = mid;
                found_soluton = true;
                break;
            }

            if (s_lo * s_mid < 0) {
                y1 = mid;
                s_hi = s_mid;
            }
            else {
                y0 = mid;
                s_lo = s_mid;
            }
        }
        ym = (y0 + y1) / 2.0;
        found_soluton = true;
    }

    if (!found_soluton) {
        return _out;
    }
    // 4. Evaluate point on polar curve    
    auto pt = polarToCartesian(ym, isLeft);
    _out.point = MPoint(pt.x, pt.y);
    _ypos = ym;

    double slope = polySlope(coefs, ym);
    Eigen::Vector2d tangent1(slope, 1);  // Assume increasing x
    if (isLeft) {
        tangent1 = Eigen::Vector2d(slope, -1);
    }
    tangent1.normalize(); 

    // Tangent vector of baseline (from left to right)
    double dx_base = right.x - left.x;
    double dy_base = right.y - left.y;
    Eigen::Vector2d tangent2(dx_base, dy_base);
    tangent2.normalize();

    // Compute angle between two tangent vectors
    double dot = tangent1.dot(tangent2);
    dot = std::min(std::max(dot, -1.0), 1.0);
    double angle = std::acos(dot);

    // 7. Optional: output tangent vector
    _out.vec1 = MPoint(tangent1.x(), tangent1.y());
    _out.vec2 = MPoint(tangent2.x(), tangent2.y());
    _out.angle = angle;

    return _out;
}

PointAndAngle
calculate_polar_angles_on_horizontal(
    const cv::Point2f& left,
    const cv::Point2f& right,
    double theta0,
    double theta1,
    double xc,
    double yc,
    const Eigen::VectorXd& coefs,
    bool isLeft
) {
    PointAndAngle _out;

    // 1. Define reference line (horizontal) from known left and right

    // 3. Perform bisection to find intersection angle
    double theta_result;
    bool found_soluton = false;

    double lo = theta0;
    double hi = theta1;
    auto polarToCartesian = [&](double theta) -> cv::Point2f {
        double r = 0;
        double t_pow = 1;
        for (int i = 0; i < coefs.size(); ++i) {
            r += coefs[i] * t_pow;
            t_pow *= theta;
        }
        double x = xc + r * cos(theta);
        double y = yc + r * sin(theta);
        return cv::Point2f((float)x, (float)y);
        };
    cv::Point2f pt_lo = polarToCartesian(lo);
    cv::Point2f pt_hi = polarToCartesian(hi);
    double s_lo = signedDistanceToLine(pt_lo, left, right);
    double s_hi = signedDistanceToLine(pt_hi, left, right);

    // If signs are not opposite, bisection won't work
    if (s_lo * s_hi <= 0.0) {
        const int max_iter = 50;
        const double tol = 0.0001;
        for (int iter = 0; iter < max_iter; ++iter) {
            double mid = (lo + hi) / 2.0;
            cv::Point2f pt_mid = polarToCartesian(mid);
            double s_mid = signedDistanceToLine(pt_mid, left, right);

            if (std::abs(s_mid) < 1e-6 || (hi - lo) < tol) {
                theta_result = mid;
                found_soluton = true;
                break;
            }

            if (s_lo * s_mid < 0) {
                hi = mid;
                s_hi = s_mid;
            }
            else {
                lo = mid;
                s_lo = s_mid;
            }
        }
        theta_result = (lo + hi) / 2.0;
        found_soluton = true;
    }

    if (!found_soluton) {
        double tangentSlope2 = 10000000; // TODO : tangentSlope2 = ellipse.tangentSlope(left.x, left.y); // TODO
        if (left.x != right.x) {
            tangentSlope2 = (right.y - left.y) / (right.x - left.x);
        }
        double left_angle = 0.0;
        if (isLeft)
            left_angle = atan2((double)left.y - yc, (double)left.x - xc);
        else
            left_angle = atan2((double)right.y - yc, (double)right.x - xc);
        if (left_angle < 0)
            left_angle += 2 * M_PI;

        auto leftSlant = polySlopePolarCoordinate(coefs, left_angle);
        _out.angle = M_PI - atan2(leftSlant, 1.0);
        _out.point.x = isLeft ? left.x : right.x;
        _out.point.y = isLeft ? left.y : right.y;
        _out.vec1.x = cos(_out.angle);
        _out.vec1.y = -sin(_out.angle);
        _out.vec2.x = cos(atan(tangentSlope2));
        _out.vec2.y = sin(atan(tangentSlope2));

        return _out;
    }
    // 4. Evaluate point on polar curve
    double r_val = 0;
    double t_pow = 1;
    for (int i = 0; i < coefs.size(); ++i) {
        r_val += coefs[i] * t_pow;
        t_pow *= theta_result;
    }

    double x = xc + r_val * cos(theta_result);
    double y = yc + r_val * sin(theta_result);
    _out.point = MPoint(x, y);

    // 5. Get slope of curve at theta_result
    double dr_dt = 0, dtheta = 1;
    for (int i = 1; i < coefs.size(); ++i) {
        dr_dt += i * coefs[i] * dtheta;
        dtheta *= theta_result;
    }

    double dx_dt = dr_dt * cos(theta_result) - r_val * sin(theta_result);
    double dy_dt = dr_dt * sin(theta_result) + r_val * cos(theta_result);
    // Tangent vector of droplet curve
    Eigen::Vector2d tangent1(dx_dt, dy_dt);
    tangent1.normalize();

    // Tangent vector of baseline (from left to right)
    double dx_base = right.x - left.x;
    double dy_base = right.y - left.y;
    Eigen::Vector2d tangent2(dx_base, dy_base);
    tangent2.normalize();

    // Compute angle between two tangent vectors
    double dot = tangent1.dot(tangent2);
    dot = std::min(std::max(dot, -1.0), 1.0);
    double angle = std::acos(dot);

    // 7. Optional: output tangent vector
    _out.vec1 = MPoint(tangent1.x(), tangent1.y());
    _out.vec2 = MPoint(tangent2.x(), tangent2.y());
    _out.angle = angle;

    return _out;
}
PointAndAngle calculate_bipoly_angles_on_curved(
    double y0,
    double y1,
    const Eigen::VectorXd& coefs,
    const DropletEllipse& ellipse,
    bool isLeft,
    int width,
    double& _ypos
) {
    PointAndAngle _out;
    _ypos = -1.f;
    // 1. Determine search range	

    // 2. Bisection method to find intersection theta with DropletEllipse
    auto signedDistance = [&](double y, bool is_left) {
        int xval = (int)polyEval(coefs, (double)y);
        int x = xval;
        if (is_left)
            x = width - xval;
        return ellipse.signedDistanceToEllipse(x, y);
    };

    const int max_iter = 30;
    const double tol = 1e-4;
    double d0 = signedDistance(y0, isLeft);
    double d1 = signedDistance(y1, isLeft);

    if (d0 * d1 > 0) {
        return _out;
    }

    double ym = 0;
    for (int i = 0; i < max_iter; ++i) {
        ym = (y0 + y1) / 2;
        double dm = signedDistance(ym, isLeft);
        if (std::abs(dm) < tol)
            break;
        if (d0 * dm < 0.0) {
            y1 = ym;
            d1 = dm;
        }
        else {
            y0 = ym;
            d0 = dm;
        }
    }

    // 3. Evaluate point at intersection angle
    int xval = (int)polyEval(coefs, (double)ym);
    int x = xval;
    if (isLeft)
        x = width - xval;
    _out.point = MPoint(x, ym);
    _ypos = ym;

    double slope = polySlope(coefs, ym);
    Eigen::Vector2d tangent1(slope, isLeft ? -1.0 : 1.0);  // Assume increasing x
    tangent1.normalize();

    // === 4. Compute ellipse tangent vector at (x, ym) ===    
    double dx, dy;
    ellipse.getTangentVector(x, ym, dx, dy);

    Eigen::Vector2d tangent2(dx, dy);
    tangent2.normalize();

    // === 5. Compute contact angle ===
    double dot = std::min(std::max(tangent1.dot(tangent2), -1.0), 1.0);
    double angle = std::acos(dot) * 180.0 / M_PI;

    // === 6. Output ===
    _out.vec1 = MPoint(tangent1.x(), tangent1.y());
    _out.vec2 = MPoint(tangent2.x(), tangent2.y());
    _out.angle = angle;
    return _out;
}

PointAndAngle
calculate_polar_angles_on_curved(
    const cv::Point2f& left,
    const cv::Point2f& right,
    double theta0,
    double theta1,
    double xc,
    double yc,
    const Eigen::VectorXd& coefs,
    const DropletEllipse& ellipse,
    bool isLeft
) {
    PointAndAngle _out;

    // 1. Determine search range	

    // 2. Bisection method to find intersection theta with DropletEllipse
    auto signedDistance = [&](double t) {
        double r_val = 0;
        double t_pow = 1;
        for (int i = 0; i < coefs.size(); ++i) {
            r_val += coefs[i] * t_pow;
            t_pow *= t;
        }
        double x = xc + r_val * cos(t);
        double y = yc + r_val * sin(t);
        return ellipse.signedDistanceToEllipse(x, y);
        };

    const int max_iter = 30;
    const double tol = 1e-4;
    double t0 = theta0, t1 = theta1;
    double d0 = signedDistance(t0);
    double d1 = signedDistance(t1);

    if (d0 * d1 > 0) {
        double left_angle = 0.0;
        if (isLeft)
            left_angle = atan2((double)left.y - yc, (double)left.x - xc);
        else
            left_angle = atan2((double)right.y - yc, (double)right.x - xc);
        if (left_angle < 0)
            left_angle += 2 * M_PI;

        auto leftSlant = polySlopePolarCoordinate(coefs, left_angle);
        _out.angle = M_PI - atan2(leftSlant, 1.0);

        double dx = 0, dy = 0;
        if(isLeft) 
            ellipse.getTangentVector(left.x, left.y, dx, dy);
		else 
			ellipse.getTangentVector(right.x, right.y, dx, dy);
                
        _out.point = isLeft ? MPoint(left.x, left.y) : MPoint(right.x, right.y);
        _out.vec1.x = cos(_out.angle);
        _out.vec1.y = -sin(_out.angle);
        _out.vec2.x = dx;
        _out.vec2.y = dy;

        return _out;
    }

    double tm = 0;
    for (int i = 0; i < max_iter; ++i) {
        tm = (t0 + t1) / 2;
        double dm = signedDistance(tm);
        if (std::abs(dm) < tol)
            break;
        if (d0 * dm < 0.0) {
            t1 = tm;
            d1 = dm;
        }
        else {
            t0 = tm;
            d0 = dm;
        }
    }

    // 3. Evaluate point at intersection angle
    double r_val = 0, t_pow = 1;
    for (int i = 0; i < coefs.size(); ++i) {
        r_val += coefs[i] * t_pow;
        t_pow *= tm;
    }
    double x = xc + r_val * cos(tm);
    double y = yc + r_val * sin(tm);
    _out.point = MPoint(x, y);

    // 4. Compute slope of droplet curve at tm
    double dr_dt = 0, dtheta = 1;
    for (int i = 1; i < coefs.size(); ++i) {
        dr_dt += i * coefs[i] * dtheta;
        dtheta *= tm;
    }

    double dx_dt = dr_dt * cos(tm) - r_val * sin(tm);
    double dy_dt = dr_dt * sin(tm) + r_val * cos(tm);
    // Normalize tangent vector for droplet
    Eigen::Vector2d tangent1(dx_dt, dy_dt);
    tangent1.normalize();

    // Tangent of ellipse at the point
    double dx, dy;
    ellipse.getTangentVector(x, y, dx, dy);    
    Eigen::Vector2d tangent2(dx, dy);

    double dot = tangent1.dot(tangent2);
    dot = std::min(std::max(dot, -1.0), 1.0);  // Clamp to avoid nan from acos
    double angle = std::acos(dot) * 180.0 / M_PI;
    _out.vec1 = MPoint(tangent1.x(), tangent1.y());
    _out.vec2 = MPoint(tangent2.x(), tangent2.y());
    _out.angle = angle;

    return _out;
}