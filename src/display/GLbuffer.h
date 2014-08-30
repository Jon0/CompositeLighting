/*
 * Buffer.h
 *
 *  Created on: 28/09/2013
 *      Author: remnanjona
 */

#ifndef GLBUFFER_H_
#define GLBUFFER_H_

#include <vector>
#include <functional>

namespace std {

template<class T> class GLbuffer {
public:
	GLenum type;
	GLuint location;
	T *data;

	/*
	 * require some function returning how many items T uses
	 */
	function<GLsizeiptr()> sizeFunc;

	GLbuffer(GLenum t, bool init) {
		type = t;
		glGenBuffers(1, &location);

		if (init) {
			data = new T();
			sizeFunc = []() -> GLsizeiptr { return 1; };
		}
	}

	GLbuffer(GLenum t, T *initial, function<GLsizeiptr()> f) {
		type = t;
		glGenBuffers(1, &location);
		insert(initial, f);
	}

	GLbuffer(GLenum t, vector<T> array):
			GLbuffer(t) {
		insert(array);
	}

	virtual ~GLbuffer() {
		glDeleteBuffers(1, &location);
	}

	void insert(T *initial, function<GLsizeiptr()> f) {
		data = initial;
		sizeFunc = f;

		// initialise buffer data
		glBindBuffer(type, location);
		glBufferData(type, sizeFunc() * sizeof(T), data, GL_STATIC_DRAW);
		glBindBuffer(type, 0);
	}

	void insert(vector<T> array) {
		insert(array.data(), [array]() -> GLsizeiptr { return array.size(); });
	}

	void update() {
		glBindBuffer(type, location);
		glBufferData(type, sizeFunc() * sizeof(T), data, GL_DYNAMIC_DRAW);
		glBindBuffer(type, 0);
	}

	void bind(GLuint bindingPoint) {
		glBindBufferBase(type, bindingPoint, location); //bindBufferRange...
	}
};

} /* namespace std */
#endif /* GLBUFFER_H_ */
