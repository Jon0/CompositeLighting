/*
 * Camera.h
 *
 *  Created on: 25/08/2013
 *      Author: remnanjona
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

#include "Modifiable.h"

namespace std {

inline optix::float3 tof3(glm::vec3 v) {
	return optix::make_float3(v.x, v.y, v.z);
}

inline float DtoR(float d) {
	return d * (static_cast<float>(M_PI) / 180.f);
}

inline float RtoD(float r) {
	return r * (180.f / static_cast<float>(M_PI));
}

void getArc(int, int, int, int, float, glm::quat &);
void getUnitCircle(int, int, int, int, glm::quat &);

class Camera: public Modifiable {
public:
	Camera();
	virtual ~Camera();

	inline bool isModified() {
		return modified;
	}

	inline void getEyeUVW(optix::float3 &e, optix::float3 &u, optix::float3 &v, optix::float3 &w) {
		//return camera->getEyeUVW(e, u, v, w);
		e = tof3(eye);
		u = tof3(U);
		v = tof3(V);
		w = tof3(W);
	}

	void alignAngle();
	void setPos(glm::vec3);
	void setAngle(glm::quat);
	void setZoom(float);
	void setFov(float);
	void reset();
	void update( float );
	void resize(int, int);

	void printAngle();
	void mouseDragRotation(int, int, int, int);
	void mouseDragPanning(int, int);

	glm::quat cameraAngle();
	float cameraExposure();
	void modifyExposure(float);

	virtual void zoom(float);
	virtual void move(glm::vec3);
	virtual void rotate(glm::quat);

private:
	bool modified;

	/*
	 * point the camera looks at
	 */
	glm::vec3 default_focus, focus, eye, U, V, W;

	glm::quat cam_angle, cam_angle_d, click_old, click_new;
	int windowwidth, windowheight;
	float viewzoom, cam_aspect, arcball_radius, arcball_x, arcball_y, hfov, vfov, exposure;
};

} /* namespace std */
#endif /* CAMERA_H_ */
