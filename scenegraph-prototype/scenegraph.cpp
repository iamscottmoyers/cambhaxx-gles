#include <iostream>

#include "scenegraph.h"

// ----------- SCENE GRAPH -------------
SceneGraphNode *SceneGraph::getRoot(void)
{
	return &root;
}

void SceneGraph::draw(void)
{
	std::cout << "//glBegin()" << std::endl;
	root.draw();
	std::cout << "//glEnd()" << std::endl;
}

void SceneGraph::outputDot(void)
{
	std::cout << "digraph SceneGraph {" << std::endl;
	root.outputDot();
	std::cout << "}" << std::endl;
}

// ----------- SCENE GRAPH NODE -------------
SceneGraphNode::iterator SceneGraphNode::begin(void)
{
	return m_children.begin();
}

SceneGraphNode::iterator SceneGraphNode::end(void)
{
	return m_children.end();
}

SceneGraphNode::SceneGraphNode(SceneGraphNodeType type)
{
	m_type = type;
}

SceneGraphNode::~SceneGraphNode(void)
{
	for(iterator i = begin(); i != end(); ++i ) {
		delete *i;
	}
}

void SceneGraphNode::addChild(SceneGraphNode *child)
{
	m_children.push_back(child);
}

void SceneGraphNode::removeChild(SceneGraphNode *child)
{
	// Note: This implementation of removeChild requires the STL to search through
	// the children list to find and remove child. It might be possible to optimise
	// this if we tend to have access to the iterator for the child we're removing
	// before calling this function.
	m_children.remove(child);
}

SceneGraphNodeType SceneGraphNode::getType(void)
{
	return m_type;
}

void SceneGraphNode::draw(void)
{
	std::cout << "SceneGraphNode :(" << std::endl;

	for(iterator i = begin(); i != end(); ++i) {
		(*i)->draw();
	}
}

void SceneGraphNode::outputDot(void)
{
	const char *name[] = {"ROOT", "TRANSLATE", "ROTATE", "VOXEL_OBJECT"};
	for(iterator i = begin(); i != end(); ++i ) {
		SceneGraphNode *child = *i;
		std::cout << "\t" << name[m_type] << "_" << this
		          << " -> " << name[child->getType()] << "_" << child
		          << std::endl;
	}

	for(iterator i = begin(); i != end(); ++i) {
		(*i)->outputDot();
	}
}

// ----------- SCENE GRAPH VOXEL OBJECT -------------
SceneGraphVoxelObject::SceneGraphVoxelObject(struct vox_t const *voxels, unsigned int num_voxels)
  : SceneGraphNode(SCENE_GRAPH_VOXEL_OBJECT)
{
	m_voxels = new struct vox_t[num_voxels];
	memcpy(m_voxels, voxels, sizeof(struct vox_t) * num_voxels);
}

SceneGraphVoxelObject::~SceneGraphVoxelObject(void)
{
	delete[] m_voxels;

	for(iterator i = begin(); i != end(); ++i ) {
		delete *i;
	}
}

void SceneGraphVoxelObject::draw(void)
{
	std::cout << "//drawDude();" << std::endl;

	for(iterator i = begin(); i != end(); ++i) {
		(*i)->draw();
	}
}

// ----------- SCENE GRAPH ROTATE -------------
SceneGraphRotate::SceneGraphRotate(void) : SceneGraphNode(SCENE_GRAPH_ROTATE)
{
}

void SceneGraphRotate::draw(void)
{
	std::cout << "//glPushMatrix();" << std::endl;
	std::cout << "//glRotate();" << std::endl;

	for(iterator i = begin(); i != end(); ++i) {
		(*i)->draw();
	}
	std::cout << "//glPopMatrix();" << std::endl;
}

// ----------- SCENE GRAPH TRANSLATE -------------
SceneGraphTranslate::SceneGraphTranslate(void) : SceneGraphNode(SCENE_GRAPH_TRANSLATE)
{
}

void SceneGraphTranslate::draw(void)
{
	std::cout << "//glPushMatrix();" << std::endl;
	std::cout << "//glTranslate();" << std::endl;

	for(iterator i = begin(); i != end(); ++i) {
		(*i)->draw();
	}
	std::cout << "//glPopMatrix();" << std::endl;
}

// ----------- SCENE GRAPH ROOT -------------
SceneGraphRoot::SceneGraphRoot(void) : SceneGraphNode(SCENE_GRAPH_ROOT)
{
}

void SceneGraphRoot::draw(void)
{
	std::cout << "//glRootDoesntDoAnything();" << std::endl;

	for(iterator i = begin(); i != end(); ++i) {
		(*i)->draw();
	}
}

int main()
{
	SceneGraph *graph;
	SceneGraphNode *root;
	SceneGraphNode *rotate;
	SceneGraphNode *translate;
	SceneGraphVoxelObject *object0;
	SceneGraphVoxelObject *object1;
	SceneGraphVoxelObject *object2;

	struct vox_t test[2] = {
		{{1, 2, 3}, {4, 5, 6, 7}},
		{{8, 9, 10}, {11, 12, 13, 14}}
	};

	graph = new SceneGraph();
	rotate = new SceneGraphRotate();
	translate = new SceneGraphTranslate();
	object0 = new SceneGraphVoxelObject(test, 2);
	object1 = new SceneGraphVoxelObject(test, 2);
	object2 = new SceneGraphVoxelObject(test, 2);

	root = graph->getRoot();
	root->addChild(rotate);
	rotate->addChild(translate);

	// TODO: Make nodes reference counted so objects can be reused as children.
	// The scenegraph should be a DAG not a tree.
	// If we have lots of duplicated objects with different translations this may save us a lot of memory.
	rotate->addChild(object0);
	translate->addChild(object1);
	translate->addChild(object2);

	graph->draw();
	graph->outputDot();

	delete graph;
}
