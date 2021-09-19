#include "A_RecursiveShapeScene.h"

A_RecursiveShapeScene::A_RecursiveShapeScene(std::vector<Point>& initialShape, int minIterations, int maxIterations)
	: initialShape_(initialShape)
	, currentIterations_(minIterations)
	, minIterations_(minIterations)
	, maxIterations_(maxIterations)
{
}

size_t A_RecursiveShapeScene::numVertices()
{
	return vertices_.size();
}

std::vector<Point> A_RecursiveShapeScene::vertices()
{
	return vertices_;
}

bool A_RecursiveShapeScene::incrementIterations()
{
	if (currentIterations_ < maxIterations_)
	{
		currentIterations_++;
		vertices_.clear();
		findShapesRecursively(initialShape_, currentIterations_);
		return true;
	}
	return false;
}

bool A_RecursiveShapeScene::decrementIterations()
{
	if (currentIterations_ > minIterations_)
	{
		currentIterations_--;
		vertices_.clear();
		findShapesRecursively(initialShape_, currentIterations_);
		return true;
	}
	return false;
}
