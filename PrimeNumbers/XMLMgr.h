#pragma once
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>


//XML Manager class. 
//Designed for work with an XML files. 
class CXMLMgr	
{
public:

	//XML data in current implementation is presented as tree.
	//Tree consists of nodes. Every node consists of tag name, it's content, pointer to the top node and container of child nodes.
	//Every node can create new child nodes, modify own data, build XML text of all nodes it consists of and be destroyed.
	class XML_Tree_Node
	{
	private:
		std::string _tagName;	//Contains name of current XML tag (without brackets)
		std::string _value;		//Content of current tag
		XML_Tree_Node* _top;	//Pointer to the top node (Parent node). If it equals nullptr - this is root node.
		std::vector<XML_Tree_Node> _nodes;	//Container of all child nodes
		
		//Special iterator for iterating own child nodes and remember last iterated node
		//It's necessary for traversing through all XML tree
		//This value resets to begin every time when some node was added or removed
		std::vector<XML_Tree_Node>::iterator _it;	

		//XML Tree nodes are stored in std::vector container.
		//_top pointer points on member of std::vector node
		//When some nodes was added or destroyed all vector data will be reorganized and all old _top pointers of it's members will be invalid.
		//Thid method recursively updates all pointers to the top nodes of current child nodes to valid values. And pointers of every node of them deeper
		void _UpdateTopNodes();

	public:

		//C-tors
		XML_Tree_Node();
		XML_Tree_Node(XML_Tree_Node* topNode, std::string Name, std::string Value);
		
		//Nothing to destroy, d-tor not needed
		//~XML_Tree_Node(); 

		//Copy c-tors, it's necessary for std::vector's push_back()
		XML_Tree_Node(const XML_Tree_Node& copy);
		XML_Tree_Node & operator=(const XML_Tree_Node& copy);

		std::string GetTagName() const;	//Get name of current tag
		std::string GetContent() const; //Get content of current tag as string. 
		//Get content of current tag as some numeric data
		template<class T> T GetContent() const 
		{
			//Making string value numeric
			std::stringstream ss; 
			T t;
			ss << _value;
			ss >> t;
			return t;
		}
		size_t GetNodeCount() const;	//How much child nodes current node has
		bool HasContent() const;		//Do this node actualy has any content (not child nodes!)

		void SetTagName(std::string tagName);	//Set tag name of current node
		void SetContent(std::string content);	//Assign new content
		
		//Assign new content value as numeric
		template<class T> void SetContent(T content)
		{
			//Making numeric value string
			std::stringstream ss;
			ss << content;
			_value = ss.str();
		}

		std::string BuildXMLOutput();	//Builds XML text of all XML deeper in this node
		XML_Tree_Node* InsertNode(std::string tagName, std::string content = "");	//Insert node with specific name and content as string
		
		//Insert node with specific name and content as numeric value
		template<class T> XML_Tree_Node* InsertNode(std::string tagName, T content)
		{
			if(tagName.length() == 0) //We cannot create a nameless tag
				throw std::runtime_error("XML Error: Cannot create a nameless tag.");
			std::stringstream ss; 
			ss << content; //Converting numeric data to string
			//Push back to node container new tag with 'this' top pointer.
			_nodes.push_back(CXMLMgr::XML_Tree_Node(this, tagName, ss.str()));	
			_it = _nodes.begin();  //Reset iterator to begin
			_UpdateTopNodes(); //Updating top node pointers
			_value = ""; //Reset current content value, tags with another tags with-in cannot have any content.
			return GetLastNode(); //Return pointer to last created node
		}

		bool Destroy();	//Destroy current node, returns true if destroyed successfuly. Cannot destroy root node.
		void RemoveAllNodes(); //Removes all child nodes
		void RemoveNode(size_t i);	//Removes one of child nodes by index
		void RemoveNode(std::string tagName, size_t skipMatches = 0);	//Removes one of child nodes by it's name and number of matches (N tag with name 'tagName')
		XML_Tree_Node* GetTopNode();	//Returns pointer to the top node (Parent node)
		XML_Tree_Node* GetLastNode();	//Returns pointer to the last child node
		XML_Tree_Node* GetNodeByIndex(size_t i);	//Returns pointer to child node by index
		XML_Tree_Node* GetNode(std::string tagName, size_t skipMatches); //Returns pointer to child node matched by name (can skip some occurrences)
		XML_Tree_Node* GetNode(std::string tagName, bool createIfNotExist = true);	//Get first matching node by name and create it if not exists (Or throw an exeption it not)
		
		//Iterates all child nodes. First call (or call with reset = true) will return first node. Every next call will return next node until it finaly will reach end.
		//When no child nodes or end is reached - returns nullptr
		XML_Tree_Node* IterateNodes(bool reset = false);

		//Returns last returned node by IterateNodes() method. If IterateNodes() wasn't called - returns first node and nullptr if no child nodes.
		//If IterateNodes() reached end it returns last existing node.
		//If some child nodes was added/removed - iterator will be reset to first node.
		XML_Tree_Node* LastIteratedNode();	

		XML_Tree_Node* operator[](size_t i);	//Return child node by inded
		XML_Tree_Node* operator[](std::string tagName);	//Return child node matching by name. If not found - new node will be created.
	};
	//End of XML_Tree_Node class

private:
	std::string m_fileName;	//Name of last used file
	XML_Tree_Node m_TreeRoot;	//Root node of XML tree

	//Parsed file (m_fileName) and generates XML tree of it. Throws runtime exeption if errors found (With information about error)
	void ParseFile();	

	//No copy constructors allowed
	CXMLMgr(const CXMLMgr& copy) = delete;	
	CXMLMgr& operator=(const CXMLMgr& node) = delete;
public:
	//Default c-tor
	CXMLMgr();	
	CXMLMgr(std::string fileName);	//Construct instance of XML manager and generate XML tree from specific file
	//Nothing to destroy, d-tor not needed
	//~CXMLMgr();

	XML_Tree_Node* GetRootNode();	//Returns pointer to the root node

	void LoadFrom(std::string fileName); //Load XML from specific file
	void Reload();	//Reload XML from last used file
	void Save();	//Save XML to last used file
	void SaveTo(std::string fileName);	//Save XML to specific file
};