#pragma once
#include <iostream>
#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <glew.h>
#include "math_func.h"



struct vertex {
	std::vector<float> v;
	void normalize() {
		float magnitude = 0.0f;
		for (int i = 0; i < v.size(); i++)
			magnitude += pow(v[i], 2.0f);
		magnitude = sqrt(magnitude);
		for (int i = 0; i < v.size(); i++)
			v[i] /= magnitude;
	}
	vertex operator-(vertex v2) {
		vertex v3;
		if (v.size() != v2.v.size()) {
			v3.v.push_back(0.0f);
			v3.v.push_back(0.0f);
			v3.v.push_back(0.0f);
		}
		else {
			for (int i = 0; i < v.size(); i++)
				v3.v.push_back(v[i] - v2.v[i]);
		}
		return v3;
	}
	vertex cross(vertex v2) {
		vertex v3;
		if (v.size() != 3 || v2.v.size() != 3) {
			v3.v.push_back(0.0f);
			v3.v.push_back(0.0f);
			v3.v.push_back(0.0f);
		}
		else {
			v3.v.push_back(v[1] * v2.v[2] - v[2] * v2.v[1]);
			v3.v.push_back(v[2] * v2.v[0] - v[0] * v2.v[2]);
			v3.v.push_back(v[0] * v2.v[1] - v[1] * v2.v[0]);
		}
		return v3;
	}
};



struct face {
	std::vector<int> vertex;
	std::vector<int> texture;
	std::vector<int> normal;
};



class LoadObj {
private:
	std::vector<vertex> vertices;
	std::vector<vertex> normals;
	std::vector<vertex> texcoords;
	std::vector<vertex> parameters;
	std::vector<face> faces;
	int numvertices;
	std::vector<float> floatVertices;
	std::vector<float> floatNormals;
	std::vector<float> floatTex;
protected:
public:
	LoadObj(std::string filename);
	float* getVertices();
	float* getNormals();
	float* getTexcoords();
	int getNumVertices();
};