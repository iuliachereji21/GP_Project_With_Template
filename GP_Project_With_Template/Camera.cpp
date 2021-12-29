#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));

    }
    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }

    glm::vec3 Camera::getCameraUpDirection() {
        return this->cameraUpDirection;
    }

    glm::vec3 Camera::getCameraTarget() {
        return this->cameraTarget;
    }

    glm::vec3 Camera::getCameraFrontDirection() {
        return this->cameraFrontDirection;
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        switch (direction) {
        case MOVE_FORWARD:
            this->cameraPosition += this->cameraFrontDirection * speed;
            this->cameraTarget += this->cameraFrontDirection * speed;
            this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
            break;

        case MOVE_BACKWARD:
            this->cameraPosition -= this->cameraFrontDirection * speed;
            this->cameraTarget -= this->cameraFrontDirection * speed;
            this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
            break;

        case MOVE_RIGHT:
            this->cameraPosition += this->cameraRightDirection * speed;
            this->cameraTarget += this->cameraRightDirection * speed;
            this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
            break;

        case MOVE_LEFT:
            this->cameraPosition -= this->cameraRightDirection * speed;
            this->cameraTarget -= this->cameraRightDirection * speed;
            this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
            break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        float dot = glm::dot(cameraFrontDirection, glm::vec3(0, 1, 0));
        float angle = glm::degrees(glm::acos(dot));
        if (angle > 120 && pitch > 0)
            pitch = 0;
        if (angle < 60 && pitch < 0)
            pitch = 0;
        cameraFrontDirection = glm::rotate(glm::mat4(1.0f), yaw, glm::cross(cameraRightDirection, cameraFrontDirection)) * glm::vec4(cameraFrontDirection, 0.0f);
        cameraFrontDirection = glm::rotate(glm::mat4(1.0f), -pitch, cameraRightDirection) * glm::vec4(cameraFrontDirection, 0.0f);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraTarget = cameraPosition + cameraFrontDirection;
    }
}