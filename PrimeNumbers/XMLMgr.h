#pragma once
#include <string>
#include <vector>
#include <map>

class XML_Tree_Node
{
private:
	friend class std::vector<XML_Tree_Node>;
	std::string _tagName;
	std::string _value;
	XML_Tree_Node* _top;
	std::vector<XML_Tree_Node> _nodes;
	std::vector<XML_Tree_Node>::iterator _it;

	void _UpdateTopNodes();

public:

	XML_Tree_Node();
	XML_Tree_Node(XML_Tree_Node* topNode, std::string Name, std::string Value);

	//For std::vector only
	XML_Tree_Node(const XML_Tree_Node& copy);
	XML_Tree_Node & operator=(const XML_Tree_Node& copy);

	std::string GetTagName() const;
	std::string GetContent() const;
	size_t GetNodeCount() const;

	void SetTagName(std::string tagName);
	void SetContent(std::string content);

	std::string BuildXMLOutput();
	XML_Tree_Node* InsertEmptyNode();
	bool Destroy();
	void RemoveNode(size_t i);
	void RemoveNode(std::string tagName, size_t skipMatches = 0);
	XML_Tree_Node* GetTopNode();
	XML_Tree_Node* GetLastNode();
	XML_Tree_Node* GetNode(size_t i);
	XML_Tree_Node* GetNode(std::string tagName, size_t skipMatches = 0);
	XML_Tree_Node* GetNodeByPath(std::string path);
	XML_Tree_Node* IterateNodes(bool reset = false);
	XML_Tree_Node* LastIteratedNode();

	XML_Tree_Node* operator[](size_t i);
	XML_Tree_Node* operator[](std::string tagName);
};

class CXMLMgr
{

private:
	std::string m_fileName;
	XML_Tree_Node m_TreeRoot;

	void ParseFile();

	CXMLMgr(const CXMLMgr& copy) = delete;
	CXMLMgr& operator=(const CXMLMgr& node) = delete;
public:
	CXMLMgr();
	CXMLMgr(std::string fileName);
	~CXMLMgr();

	std::string GetContent (std::string tagSequence, size_t n = 0) const;

	void LoadFrom(std::string fileName);
	void Reload();
	void Save();
	void SaveTo(std::string fileName);
};