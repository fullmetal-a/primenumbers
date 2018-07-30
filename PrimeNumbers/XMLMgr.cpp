#include "XMLMgr.h"
#include <stdexcept>
#include <io.h>
#include <fstream>
#include <regex>
#include <iostream>
#include <stack>
#include <vector>


std::string formTag(std::string tagName, bool isClosing)
{
	std::string retval = "<";
	if (isClosing)
		retval += "/";
	retval += tagName + ">";
	return retval;
}

bool CheckTagName(std::string tagName)
{
	std::regex tagName_re("^[A-Za-z0-9_-]+$");

	if (tagName.length() == 0)
		return false;

	if (tagName.length() > 1000)
		return false;

	if (!std::regex_match(tagName, tagName_re))
		return false;
	else
		return true;
}

CXMLMgr::XML_Tree_Node::XML_Tree_Node()
{
	_tagName = "";
	_value = "";
	_top = nullptr;
	_it = _nodes.begin();
}

CXMLMgr::XML_Tree_Node::XML_Tree_Node(CXMLMgr::XML_Tree_Node* topNode, std::string Name, std::string Value) :
	_tagName(Name),
	_value(Value),
	_top(topNode)
{
	_it = _nodes.begin();
}

CXMLMgr::XML_Tree_Node::XML_Tree_Node(const CXMLMgr::XML_Tree_Node& copy)
{
	_tagName = copy._tagName;
	_value = copy._value;
	_top = copy._top;
	_nodes = copy._nodes;
	_it = _nodes.begin();
}

CXMLMgr::XML_Tree_Node& CXMLMgr::XML_Tree_Node::operator=(const CXMLMgr::XML_Tree_Node& copy)
{
	_tagName = copy._tagName;
	_value = copy._value;
	_top = copy._top;
	_nodes = copy._nodes;
	_it = _nodes.begin();
	return *this;
}

std::string CXMLMgr::XML_Tree_Node::GetTagName() const
{
	return _tagName;
}

void CXMLMgr::XML_Tree_Node::SetTagName(std::string tagName)
{
	if(!CheckTagName(tagName))
		throw std::runtime_error("XML Parsing error: Bad tag name.");
	_tagName = tagName;
}

void CXMLMgr::XML_Tree_Node::_UpdateTopNodes()
{
	for (size_t i = 0; i < _nodes.size(); i++)
	{
		for (size_t j = 0; j < _nodes[i]._nodes.size(); j++)
		{
			_nodes[i]._UpdateTopNodes();
			_nodes[i]._nodes[j]._top = &_nodes[i];
		}
	}
}

bool CXMLMgr::XML_Tree_Node::Destroy()
{
	if (!_top)
		return false;

	for (auto it = _top->_nodes.begin(); it != _top->_nodes.end(); it++)
	{
		if (&(*it) == this)
		{
			_top->_nodes.erase(it);
			_top->_it = _top->_nodes.begin();
			_top->_UpdateTopNodes();
			break;
		}
	}
	return true;
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::GetLastNode()
{
	size_t size = _nodes.size();
	if (size == 0)
		return nullptr;
	else
		return &_nodes[size-1];
}

void CXMLMgr::XML_Tree_Node::RemoveNode(size_t i)
{
	_nodes.erase(_nodes.begin() + i);
	_it = _nodes.begin();
	_UpdateTopNodes();
}

void CXMLMgr::XML_Tree_Node::RemoveNode(std::string tagName, size_t skipMatches)
{
	size_t matches = 0;
	for (auto it = _nodes.begin(); it != _nodes.end(); it++)
	{
		if ((*it)._tagName == tagName)
		{
			if (matches == skipMatches)
			{
				_nodes.erase(it);
				_it = _nodes.begin();
				_UpdateTopNodes();
				break;
			}
			matches++;
		}
	}
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::GetTopNode()
{
	return _top;
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::GetNodeByIndex(size_t i)
{
	return &_nodes[i];
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::GetNode(std::string tagName, size_t skipMatches)
{
	size_t matches = 0;
	for (auto it = _nodes.begin(); it != _nodes.end(); it++)
	{
		if ((*it)._tagName == tagName)
		{
			if (matches == skipMatches)
			{
				return &(*it);
				break;
			}
			matches++;
		}
	}

	char errmsg[1200];
	sprintf_s(errmsg, 1200, "XML GetNode error: Tag \"%s\"[%u] does not exist.", tagName.c_str(), skipMatches);
	throw std::runtime_error(errmsg);
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::GetNode(std::string tagName, bool createIfNotExist)
{
	for (auto it = _nodes.begin(); it != _nodes.end(); it++)
	{
		if ((*it)._tagName == tagName)
		{
			return &(*it);
			break;
		}
	}

	if (createIfNotExist)
		return InsertNode(tagName);
	else
	{
		char errmsg[1200];
		sprintf_s(errmsg, 1200, "XML GetNode error: Tag \"%s\" does not exist.", tagName.c_str());
		throw std::runtime_error(errmsg);
	}
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::IterateNodes(bool reset)
{
	if (reset)
		_it = _nodes.begin();

	if (_nodes.size() == 0)
		return nullptr;

	if (_it == _nodes.end())
		return nullptr;

	CXMLMgr::XML_Tree_Node* node = &(*_it);
	_it++;
	return node;
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::LastIteratedNode()
{
	if (_nodes.size() == 0)
		return nullptr;
	else if (_it == _nodes.end())
		return GetLastNode();
	else if (_it == _nodes.begin())
		return &(*_it);
	else
		return &(*(_it - 1));
}

std::string CXMLMgr::XML_Tree_Node::BuildXMLOutput()
{
	if (GetNodeCount() == 0)
		return "";

	CXMLMgr::XML_Tree_Node* tmpNode, *node = this;
	std::string xml;
	size_t depth = 0;
	bool first = false;

	while (true)
	{
		auto a = node->LastIteratedNode();
		auto b = node->GetLastNode();
		if (node->GetTopNode() == nullptr && (node->GetNodeCount() == 1 && first || (node->GetNodeCount() > 1 && node->LastIteratedNode() == node->GetLastNode())))
			break;

		if(!first)
			first = true;

		if (node->GetNodeCount() > 0)	//If we can go deeper
		{
			tmpNode = node->IterateNodes();
			if (!tmpNode)
			{
				depth--;
				for (size_t i = 0; i < depth; i++)
					xml += "\t";
				xml += formTag(node->GetTagName(), true) + "\r\n";
				node = node->GetTopNode();
				continue;
			}
			else
			{
				for (size_t i = 0; i < depth; i++)
					xml += "\t";
				node = tmpNode;
				xml += formTag(node->GetTagName(), false);
				if (node->GetNodeCount() > 0)
					xml += "\r\n";
				depth++;
			}
		}
		else //If it's finish of branch
		{
			xml += node->GetContent<std::string>();
			xml += formTag(node->GetTagName(), true) + "\r\n";
			node = node->GetTopNode();
			depth--;
		}
	}
	return xml;
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::operator[](size_t i)
{
	return GetNodeByIndex(i);
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::operator[](std::string tagName)
{
	return GetNode(tagName);
}

size_t CXMLMgr::XML_Tree_Node::GetNodeCount() const
{
	return _nodes.size();
}

CXMLMgr::CXMLMgr() :
m_fileName("")
{
}

CXMLMgr::CXMLMgr(std::string fileName) :
	m_fileName(fileName)
{
	ParseFile();
}



CXMLMgr::~CXMLMgr()
{
}

void CXMLMgr::ParseFile()
{
	if (_access(m_fileName.c_str(), 0) == -1)
		return;

	std::ifstream f(m_fileName);
	std::string fileContent;
	f.seekg(0, std::ios::end);
	fileContent.reserve(size_t(f.tellg()));
	f.seekg(0, std::ios::beg);

	fileContent.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	f.close();

	
	std::string tagName, content;
	std::stack<std::pair<std::string, size_t>> openedTags;
	size_t dest = 0, offset = 0, tempOffset = 0, contentLength = 0;
	size_t fileLen = fileContent.length();
	bool closingTag = false;
	CXMLMgr::XML_Tree_Node* node = &m_TreeRoot; 

	while(true)
	{
		offset = fileContent.find("<", offset);
		tempOffset = fileContent.find("</", offset);

		if (offset == std::string::npos && tempOffset == std::string::npos)
		{
			if (openedTags.size() > 0)
			{
				m_fileName = "";
				throw std::runtime_error("XML Parsing error: Reached end of the file, but some tags isn't closed.");
			}
			break;
		}
		else
		{
			if (offset == tempOffset)
			{
				closingTag = true;
				offset++;
			}
			else
				closingTag = false;

			dest = fileContent.find(">", offset);
			if (dest == std::string::npos)
			{
				m_fileName = "";
				throw std::runtime_error("XML Parsing error: Endless tag found.");
			}

			tagName = fileContent.substr(offset + 1, dest - (offset + 1));
			if (tagName.length() == 0)
			{
				m_fileName = "";
				throw std::runtime_error("XML Parsing error: Nameless tag found.");
			}

			if(tagName.length() > 1000)
			{
				m_fileName = "";
				throw std::runtime_error("XML Parsing error: Too long tag name.");
			}

			if (!CheckTagName(tagName))
			{
				fileContent = ""; m_fileName = "";
				throw std::runtime_error("XML Parsing error: Bad tag name.");
			}

			if (closingTag)
			{
				if (tagName == openedTags.top().first)
				{
					contentLength = offset - openedTags.top().second;
					content = fileContent.substr(openedTags.top().second, contentLength);
					node->SetContent(content);
					node = node->GetTopNode();
					openedTags.pop();
				}

				else
				{
					m_fileName = "";
					throw std::runtime_error(std::string(std::string("XML Parsing error: Unexpected closing tag \"</") + tagName + ">\", expected: \"</" + openedTags.top().first + ">\"").c_str());
				}
			}
			else
			{
				openedTags.push(std::pair<std::string, size_t>(tagName, dest));
				node = node->InsertNode(tagName);
			}

			offset = dest;
		}
	}
}

void CXMLMgr::LoadFrom(std::string fileName)
{
	m_fileName = fileName;
	ParseFile();
}

void CXMLMgr::Reload()
{
	ParseFile();
}

void CXMLMgr::Save()
{
	std::ofstream f(m_fileName);
	f << m_TreeRoot.BuildXMLOutput();
	f.close();
}

void CXMLMgr::SaveTo(std::string fileName)
{
	m_fileName = fileName;
	Save();
}

CXMLMgr::XML_Tree_Node* CXMLMgr::GetRootNode()
{
	return &m_TreeRoot;
}