#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include <set>

/// @brief A type for each node
typedef enum SceneGraphNodeType {
	SCENE_GRAPH_ROOT,        //< Root node, only one of these per SceneGraph
	SCENE_GRAPH_TRANSLATE,   //< Translates all children
	SCENE_GRAPH_ROTATE,      //< Rotates all children
	SCENE_GRAPH_VOXEL_OBJECT //< An object created from voxels
} SceneGraphNodeType;

/// @brief A node in a SceneGraph
/// Scene graph nodes may have many children and many parents but must not be part of a cycle.
/// The user must not call delete on any SceneGraphNode objects created. They are a reference
/// counted object. Once the user does not need the object any more they should call the release()
/// function.
class SceneGraphNode {
private:
	SceneGraphNodeType m_type;
	std::set<SceneGraphNode *> m_children;
	int m_refcount;
public:
	// Iterator for traversing the child set
	typedef std::set<SceneGraphNode *>::iterator iterator;
	iterator begin(void);
	iterator end(void);

	SceneGraphNode(SceneGraphNodeType type);
	virtual ~SceneGraphNode(void);
	void retain(void);
	void release(void);

	SceneGraphNodeType getType(void);
	void addChild(SceneGraphNode *child);
	void removeChild(SceneGraphNode *child);

	virtual void draw(void);
	virtual void outputDot(void);
};

/// @brief Represents one voxel
struct vox_t {
	unsigned char colour[4];
	float pos[3];
};

/// @brief A voxel object
/// Contains many voxels that build up one object.
/// Voxel objects like any other node may have children so voxel objects
/// can be composed of other voxel objects that may be translated / rotated.
class SceneGraphVoxelObject : public SceneGraphNode {
private:
	struct vox_t *m_voxels;
public:
	SceneGraphVoxelObject(struct vox_t const *voxels, unsigned int num_voxels);
	virtual ~SceneGraphVoxelObject(void);
	virtual void draw(void);
};

/// @brief A translation command
/// Translates all children by a 3D vector given in voxels.
class SceneGraphTranslate : public SceneGraphNode {
private:
	unsigned int m_translation[3];
public:
	SceneGraphTranslate(unsigned int const translation[]);
	void update(unsigned int const translation[]);
	virtual void draw(void);
};

/// @brief A rotation command
/// Rotates all children by the 3D value given in degrees.
class SceneGraphRotate : public SceneGraphNode {
private:
	unsigned int m_rotation[3];
public:
	SceneGraphRotate(unsigned int const rotation[]);
	void update(unsigned int const rotation[]);
	virtual void draw(void);
};

/// @brief Root node for the scene graph
class SceneGraphRoot : public SceneGraphNode {
public:
	SceneGraphRoot(void);
	virtual void draw(void);
};

/// @brief The Scene Graph
/// SceneGraphs are directed acyclic graphs (DAGs). It is possible with the API
/// for a programmer to create a cycle. If this occurs an assertion will be triggered.
class SceneGraph {
private:
	/// The root node created and destroyed with the scene graph.
	SceneGraphRoot root;
public:
	/// Returns a reference to the root node. The root node of a graph is not a reference counted
	/// object so the caller should not call release on this node.
	SceneGraphNode *getRoot(void);

	/// Draw the scene graph using OpenGLES.
	void draw(void);

	/// Output the scene graph in Graphviz Dot format. Useful for debugging.
	void outputDot(void);
};

#endif /* SCENEGRAPH_H */
