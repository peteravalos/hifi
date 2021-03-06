//
//  ShapeInfoUtil.cpp
//  libraries/physcis/src
//
//  Created by Andrew Meadows 2014.12.01
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <SharedUtil.h> // for MILLIMETERS_PER_METER

#include "ShapeInfoUtil.h"
#include "BulletUtil.h"

int ShapeInfoUtil::toBulletShapeType(int shapeInfoType) {
    int bulletShapeType = INVALID_SHAPE_PROXYTYPE;
    switch(shapeInfoType) {
        case SHAPE_TYPE_BOX:
            bulletShapeType = BOX_SHAPE_PROXYTYPE;
            break;
        case SHAPE_TYPE_SPHERE:
            bulletShapeType = SPHERE_SHAPE_PROXYTYPE;
            break;
        case SHAPE_TYPE_CAPSULE_Y:
            bulletShapeType = CAPSULE_SHAPE_PROXYTYPE;
            break;
        case SHAPE_TYPE_CONVEX_HULL:
            bulletShapeType = CONVEX_HULL_SHAPE_PROXYTYPE;
            break;
    }
    return bulletShapeType;
}

int ShapeInfoUtil::fromBulletShapeType(int bulletShapeType) {
    int shapeInfoType = SHAPE_TYPE_NONE;
    switch(bulletShapeType) {
        case BOX_SHAPE_PROXYTYPE:
            shapeInfoType = SHAPE_TYPE_BOX;
            break;
        case SPHERE_SHAPE_PROXYTYPE:
            shapeInfoType = SHAPE_TYPE_SPHERE;
            break;
        case CAPSULE_SHAPE_PROXYTYPE:
            shapeInfoType = SHAPE_TYPE_CAPSULE_Y;
            break;
        case CONVEX_HULL_SHAPE_PROXYTYPE:
            shapeInfoType = SHAPE_TYPE_CONVEX_HULL;
            break;
    }
    return shapeInfoType;
}

void ShapeInfoUtil::collectInfoFromShape(const btCollisionShape* shape, ShapeInfo& info) {
    if (shape) {
        int type = ShapeInfoUtil::fromBulletShapeType(shape->getShapeType());
        switch(type) {
            case SHAPE_TYPE_BOX: {
                const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
                info.setBox(bulletToGLM(boxShape->getHalfExtentsWithMargin()));
            }
            break;
            case SHAPE_TYPE_SPHERE: {
                const btSphereShape* sphereShape = static_cast<const btSphereShape*>(shape);
                info.setSphere(sphereShape->getRadius());
            }
            break;
            case SHAPE_TYPE_CONVEX_HULL: {
                const btConvexHullShape* convexHullShape = static_cast<const btConvexHullShape*>(shape);
                const int numPoints = convexHullShape->getNumPoints();
                const btVector3* btPoints = convexHullShape->getUnscaledPoints();
                QVector<glm::vec3> points;
                for (int i = 0; i < numPoints; i++) {
                    glm::vec3 point(btPoints->getX(), btPoints->getY(), btPoints->getZ());
                    points << point;
                }
                info.setConvexHull(points);
            }
            break;
            default: {
                info.clear();
            }
            break;
        }
    } else {
        info.clear();
    }
}

btCollisionShape* ShapeInfoUtil::createShapeFromInfo(const ShapeInfo& info) {
    btCollisionShape* shape = NULL;
    switch(info.getType()) {
        case SHAPE_TYPE_BOX: {
            shape = new btBoxShape(glmToBullet(info.getHalfExtents()));
        }
        break;
        case SHAPE_TYPE_SPHERE: {
            float radius = info.getHalfExtents().x;
            shape = new btSphereShape(radius);
        }
        break;
        case SHAPE_TYPE_CAPSULE_Y: {
            glm::vec3 halfExtents = info.getHalfExtents();
            float radius = halfExtents.x;
            float height = 2.0f * halfExtents.y;
            shape = new btCapsuleShape(radius, height);
        }
        break;
        case SHAPE_TYPE_CONVEX_HULL: {
            shape = new btConvexHullShape();
            const QVector<glm::vec3>& points = info.getPoints();
            foreach (glm::vec3 point, points) {
                btVector3 btPoint(point[0], point[1], point[2]);
                static_cast<btConvexHullShape*>(shape)->addPoint(btPoint);
            }
        }
        break;
    }
    return shape;
}
