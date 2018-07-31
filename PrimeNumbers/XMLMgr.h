#pragma once
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>

class CXMLMgr
{
public:
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

		XML_Tree_Node(const XML_Tree_Node& copy);
		XML_Tree_Node & operator=(const XML_Tree_Node& copy);

		std::string GetTagName() const;
		std::string GetContent() const;
		template<class T> T GetContent() const
		{
			std::stringstream ss;
			T t;
			ss << _value;
			ss >> t;
			return t;
		}
		size_t GetNodeCount() const;
		bool HasContent() const;

		void SetTagName(std::string tagName);
		void SetContent(std::string content);
		template<class T> void SetContent(T content)
		{
			std::stringstream ss;
			ss << content;
			_value = ss.str();
		}

		std::string BuildXMLOutput();
		XML_Tree_Node* InsertNode(std::string tagName, std::string content = "");
		template<class T> XML_Tree_Node* InsertNode(std::string tagName, T content)
		{
			if(tagName.length() == 0)
				throw std::runtime_error("XML Error: Cannot create a nameless tag.");
			std::stringstream ss;
			ss << content;
			_nodes.push_back(CXMLMgr::XML_Tree_Node(this, tagName, ss.str()));
			_it = _nodes.begin();
			_UpdateTopNodes();
			return GetLastNode();
		}
		bool Destroy();
		void RemoveNode(size_t i);
		void RemoveNode(std::string tagName, size_t skipMatches = 0);
		XML_Tree_Node* GetTopNode();
		XML_Tree_Node* GetLastNode();
		XML_Tree_Node* GetNodeByIndex(size_t i);
		XML_Tree_Node* GetNode(std::string tagName, size_t skipMatches);
		XML_Tree_Node* GetNode(std::string tagName, bool createIfNotExist = true);
		XML_Tree_Node* IterateNodes(bool reset = false);
		XML_Tree_Node* LastIteratedNode();

		XML_Tree_Node* operator[](size_t i);
		XML_Tree_Node* operator[](std::string tagName);
	};

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

	XML_Tree_Node* GetRootNode();

	void LoadFrom(std::string fileName);
	void Reload();
	void Save();
	void SaveTo(std::string fileName);
};