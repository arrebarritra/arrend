#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>


// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
	enum Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	enum ProjectionType {
		Perspective,
		Orthogonal
	};

	struct Matrices {
		glm::mat4 projection;
		glm::mat4 view;
	};

	struct Plane
	{
		glm::vec3 point, normal;
	};

	struct Frustum
	{
		Plane topFace;
		Plane bottomFace;

		Plane rightFace;
		Plane leftFace;

		Plane farFace;
		Plane nearFace;
	};

	// camera Attributes
	float Aspect;

	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	Matrices matrices;
	bool projectionIsDirty;
	bool viewIsDirty;
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	ProjectionType projType;

	// constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, ProjectionType proj = Perspective) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		projType = proj;
		updateCameraVectors();
		CalcViewMatrix();
		CalcProjectionMatrix();
	}
	// constructor with scalar values
	Camera(glm::vec3 pos, glm::vec3 up, float yaw, float pitch, float aspect, ProjectionType proj) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = pos;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		Aspect = aspect;
		projType = proj;
		updateCameraVectors();
		CalcViewMatrix();
		CalcProjectionMatrix();
	}

	Frustum createFrustumFromCamera(const Camera& cam, float aspect, float fovY,
		float zNear, float zFar)
	{
		Frustum     frustum;
		const float halfVSide = zFar * tanf(fovY * .5f);
		const float halfHSide = halfVSide * aspect;
		const glm::vec3 frontMultFar = zFar * cam.Front;

		frustum.nearFace = { cam.Position + zNear * cam.Front, cam.Front };
		frustum.farFace = { cam.Position + frontMultFar, -cam.Front };
		frustum.rightFace = { cam.Position,
								glm::cross(frontMultFar - cam.Right * halfHSide, cam.Up) };
		frustum.leftFace = { cam.Position,
								glm::cross(cam.Up,frontMultFar + cam.Right * halfHSide) };
		frustum.topFace = { cam.Position,
								glm::cross(cam.Right, frontMultFar - cam.Up * halfVSide) };
		frustum.bottomFace = { cam.Position,
								glm::cross(frontMultFar + cam.Up * halfVSide, cam.Right) };

		return frustum;
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		CalcViewMatrix();
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
		CalcViewMatrix();
	}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
		CalcProjectionMatrix();
	}

	void framebuffer_size_callback(int width, int height) {
		Aspect = width / (float)height;
		CalcProjectionMatrix();
	}

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));

	}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	void CalcViewMatrix()
	{
		matrices.view = glm::lookAt(Position, Position + Front, Up);
		viewIsDirty = true;
	}

	// returns the projection matrix (orthogonal or perspective) given screen size
	void CalcProjectionMatrix() {
		if (projType == Perspective)
			matrices.projection = glm::perspective(glm::radians(Zoom), Aspect, 0.1f, 100.0f);
		else if (projType == Orthogonal)
			matrices.projection = glm::ortho(0.0f, Aspect, 0.0f, 1.0f);
		projectionIsDirty = true;
	}
};
#endif