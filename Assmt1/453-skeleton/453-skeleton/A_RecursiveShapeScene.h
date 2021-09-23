#pragma once

#include <vector>
#include <glm/glm.hpp>

typedef glm::vec3 Point;
typedef glm::vec3 Colour;

class A_RecursiveShapeScene {
public:
	A_RecursiveShapeScene(std::vector<Point>& initialShape, int minIterations, int maxIterations);

	size_t numVertices() const;
	std::vector<Point> vertices() const;
	bool incrementIterations();
	bool decrementIterations();

	virtual int groupingSize() const = 0;
	virtual Colour vertexColour(int vertex) = 0;

private:
	virtual void findShapesRecursively(std::vector<Point>& initialShape_, int iterations) = 0;
	virtual void pushShapesIntoVertices(std::vector<Point>& initialShape_) = 0;

protected:
	std::vector<Point> vertices_;
	int currentIterations_;

private:
	int minIterations_;
	int maxIterations_;
	std::vector<Point>& initialShape_;
};
