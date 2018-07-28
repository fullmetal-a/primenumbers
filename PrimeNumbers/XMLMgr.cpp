#include "XMLMgr.h"
#include <stdexcept>
#include <io.h>
#include <fstream>
#include <regex>
#include <iostream>
#include <stack>
#include <vector>
#include <list>

XML_Tree_Node::XML_Tree_Node()
{
	_tagName = "";
	_value = "";
	_top = nullptr;
}

XML_Tree_Node::XML_Tree_Node(XML_Tree_Node* topNode, std::string Name, std::string Value) :
	_tagName(Name),
	_value(Value),
	_top(topNode)
{
}

XML_Tree_Node::XML_Tree_Node(const XML_Tree_Node& copy)
{
	_tagName = copy._tagName;
	_value = copy._value;
	_top = copy._top;
	_nodes = copy._nodes;
}

XML_Tree_Node& XML_Tree_Node::operator=(const XML_Tree_Node& copy)
{
	_tagName = copy._tagName;
	_value = copy._value;
	_top = copy._top;
	_nodes = copy._nodes;
	return *this;
}

std::string XML_Tree_Node::GetTagName() const
{
	return _tagName;
}

std::string XML_Tree_Node::GetContent() const
{
	return _value;
}

void XML_Tree_Node::SetTagName(std::string tagName)
{
	_tagName = tagName;
}

void XML_Tree_Node::SetContent(std::string content)
{
	_value = content;
}

bool XML_Tree_Node::Destroy()
{
	if (!_top)
		return false;

	for (auto it = _top->_nodes.begin(); it != _top->_nodes.end(); it++)
	{
		if (&(*it) == this)
		{
			_top->_nodes.erase(it);
			break;
		}
	}
	return true;
}

XML_Tree_Node* XML_Tree_Node::InsertNode(std::string tagName, std::string Content)
{
	_nodes.push_back(XML_Tree_Node(this, tagName, Content));
	return &_nodes[_nodes.size() - 1];
}

void XML_Tree_Node::RemoveNode(size_t i)
{
	_nodes.erase(_nodes.begin() + i);
}

void XML_Tree_Node::RemoveNode(std::string tagName, size_t skipMatches)
{
	size_t matches = 0;
	for (auto it = _nodes.begin(); it != _nodes.end(); it++)
	{
		if ((*it)._tagName == tagName)
		{
			if (matches == skipMatches)
			{
				_nodes.erase(it);
				break;
			}
			matches++;
		}
	}
}

XML_Tree_Node* XML_Tree_Node::GetTopNode()
{
	return _top;
}

XML_Tree_Node* XML_Tree_Node::GetNode(size_t i)
{
	return &_nodes[i];
}

XML_Tree_Node* XML_Tree_Node::GetNode(std::string tagName, size_t skipMatches)
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
	return nullptr;
}

XML_Tree_Node* XML_Tree_Node::GetNodeByPath(std::string path)
{
	return nullptr;
}

std::string XML_Tree_Node::BuildXMLOutput() const
{
	return "";
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

bool CXMLMgr::CheckTagName(std::string tagName) const
{
	std::regex tagName_re("^[A-Za-z0-9_-]+$");

	if (tagName.length() == 0)
		return false;

	if (!std::regex_match(tagName, tagName_re))
		return false;
	else
		return true;
}

void CXMLMgr::ParseFile()
{
	if(_access(m_fileName.c_str(), 0) == -1)
		throw std::runtime_error("Wrong file name. File does not exist.");

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
	XML_Tree_Node* node = &m_TreeRoot;

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
				fileContent = ""; m_fileName = "";
				throw std::runtime_error("XML Parsing error: Endless tag found.");
			}

			tagName = fileContent.substr(offset + 1, dest - (offset + 1));
			if (tagName.length() == 0)
			{
				fileContent = ""; m_fileName = "";
				throw std::runtime_error("XML Parsing error: Nameless tag found.");
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
					node->SetTagName(tagName);
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
				openedTags.push(std::pair<std::string, size_t>(tagName, dest + 1));
				node = node->InsertNode("", "");
			}

			offset = dest;
		}
	}
	
}

void CXMLMgr::LoadFrom(std::string fileName)
{

}

void CXMLMgr::Reload()
{

}

void CXMLMgr::Save()
{

}

void CXMLMgr::SaveTo(std::string fileName)
{

}

std::string CXMLMgr::GetContent(std::string tagSequence, size_t n) const
{
	return "";
}