#include "A_RecursiveShapeScene.h"

A_RecursiveShapeScene::A_RecursiveShapeScene(std::vector<Point>& initialShape, int minIterations, int maxIterations)
	: currentIterations_(minIterations)
	, minIterations_(minIterations)
	, maxIterations_(maxIterations)
	, initialShape_(initialShape)
{
}

size_t A_RecursiveShapeScene::numVertices() const
{
	return vertices_.size();
}

std::vector<Point> A_RecursiveShapeScene::vertices() const
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
