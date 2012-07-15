#include <iostream>
#include <cstring>
#include <cassert>
#include <set>

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
	m_refcount = 1;
}

SceneGraphNode::~SceneGraphNode(void)
{
	for(iterator i = begin(); i != end(); ++i ) {
		(*i)->release();
	}
}

void SceneGraphNode::retain(void)
{
	assert(m_refcount > 0 && "Refcount is not greater than 0. Perhaps the object has been deleted?");
	++m_refcount;
}

void SceneGraphNode::release(void)
{
	assert(m_refcount > 0 && "Refcount is not greater than 0. Perhaps the object has been deleted?");
	--m_refcount;
	if(m_refcount <= 0) {
		/* Commit suicide. Don't use any members after this call. */
		delete this;
	}
}

static bool racyclic(SceneGraphNode *node, std::set<SceneGraphNode *> &visiting, std::set<SceneGraphNode *> &finished)
{
	// Insert this node into the visiting set
	visiting.insert(node);

	// Visit each child node
	for(SceneGraphNode::iterator i = node->begin(); i != node->end(); ++i) {
		SceneGraphNode *child = *i;

		// If the child is already finished we can ignore it as we know it has no cycles
		if(finished.find(child) == finished.end()) {

			// If the child is in the visiting set we have found a cycle
			if(visiting.find(child) != visiting.end()) {
				return false;
			}

			// If any of this nodes children are cyclic return false
			if(!racyclic(child, visiting, finished)) {
				return false;
			}
		}
	}

	// Remove from the visiting set and plase in the finished set
	visiting.erase(node);
	finished.insert(node);

	// This node is not part of a cycle
	return true;
}

static bool acyclic(SceneGraphNode *node)
{
	std::set<SceneGraphNode *> visiting;
	std::set<SceneGraphNode *> finished;

	return racyclic(node, visiting, finished);
}

void SceneGraphNode::addChild(SceneGraphNode *child)
{
	child->retain();
	m_children.push_back(child);
	assert(acyclic(child) && "addChild() created a cycle in the scene graph.");
}

void SceneGraphNode::removeChild(SceneGraphNode *child)
{
	// Note: This implementation of removeChild requires the STL to search through
	// the children list to find and remove child. It might be possible to optimise
	// this if we tend to have access to the iterator for the child we're removing
	// before calling this function.
	m_children.remove(child);
	child->release();
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
		(*i)->release();
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
SceneGraphRotate::SceneGraphRotate(unsigned int const rotation[]) : SceneGraphNode(SCENE_GRAPH_ROTATE)
{
	memcpy(m_rotation, rotation, sizeof(m_rotation));
}

void SceneGraphRotate::update(unsigned int const rotation[])
{
	memcpy(m_rotation, rotation, sizeof(m_rotation));
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
SceneGraphTranslate::SceneGraphTranslate(unsigned int const translation[]) : SceneGraphNode(SCENE_GRAPH_TRANSLATE)
{
	memcpy(m_translation, translation, sizeof(m_translation));
}

void SceneGraphTranslate::update(unsigned int const translation[])
{
	memcpy(m_translation, translation, sizeof(m_translation));
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

// ----------- MAIN -------------
int main(int argc, char *argv[])
{
	SceneGraph *graph;
	SceneGraphNode *root;
	SceneGraphNode *rotate;
	SceneGraphNode *translate;
	SceneGraphVoxelObject *object0;
	SceneGraphVoxelObject *object1;
	SceneGraphVoxelObject *object2;

	const struct vox_t test[2] = {
		{{1, 2, 3, 4}, {5, 6, 7}},
		{{8, 9, 10, 11}, {12, 13, 14}}
	};
	const unsigned int r[3] = {1, 2, 3};
	const unsigned int t[3] = {4, 5, 6};

	graph = new SceneGraph();
	rotate = new SceneGraphRotate(r);
	translate = new SceneGraphTranslate(t);
	object0 = new SceneGraphVoxelObject(test, 2);
	object1 = new SceneGraphVoxelObject(test, 2);
	object2 = new SceneGraphVoxelObject(test, 2);

	root = graph->getRoot();
	root->addChild(rotate);
	rotate->addChild(translate);

	rotate->addChild(object0);
	rotate->addChild(object2);
	rotate->release();

	translate->addChild(object1);
	translate->addChild(object2);
	translate->release();

	object0->release();
	object1->release();
	object2->release();

	graph->draw();
	graph->outputDot();

	delete graph;

	return 0;
}
