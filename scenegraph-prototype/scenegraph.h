#include <list>

typedef enum SceneGraphNodeType {
	SCENE_GRAPH_ROOT,
	SCENE_GRAPH_TRANSLATE,
	SCENE_GRAPH_ROTATE,
	SCENE_GRAPH_VOXEL_OBJECT
} SceneGraphNodeType;

class SceneGraphNode {
private:
	SceneGraphNodeType m_type;
	std::list<SceneGraphNode *> m_children;
	int m_refcount;
public:
	// Iterator for traversing the child list
	typedef std::list<SceneGraphNode *>::iterator iterator;
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

struct vox_t {
	float pos[3];
	unsigned char colour[4];
};

class SceneGraphVoxelObject : public SceneGraphNode {
private:
	struct vox_t *m_voxels;
public:
	SceneGraphVoxelObject(struct vox_t const *voxels, unsigned int num_voxels);
	virtual ~SceneGraphVoxelObject(void);
	virtual void draw(void);
};

class SceneGraphTranslate : public SceneGraphNode {
public:
	SceneGraphTranslate(void);
	virtual void draw(void);
};

class SceneGraphRotate : public SceneGraphNode {
public:
	SceneGraphRotate(void);
	virtual void draw(void);
};

class SceneGraphRoot : public SceneGraphNode {
public:
	SceneGraphRoot(void);
	virtual void draw(void);
};

class SceneGraph {
private:
	SceneGraphRoot root;
public:
	SceneGraphNode *getRoot(void);
	void draw(void);
	void outputDot(void);
};
