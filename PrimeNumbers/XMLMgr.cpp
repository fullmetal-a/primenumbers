#include "XMLMgr.h"
#include <io.h>
#include <fstream>
#include <regex>
#include <iostream>
#include <stack>
#include <vector>

//Utils 
namespace XML_Utils
{
	//Forms XML tag from tag name (<tag> or </tag>
	std::string formTag(const std::string& tagName, bool isClosing)	
	{
		std::string retval = "<";
		if (isClosing)
			retval += "/";
		retval += tagName + ">";
		return retval;
	}

	//Cheks if XML tag name is correct, returns false if not
	bool CheckTagName(const std::string& tagName)
	{
		std::regex tagName_re("^[A-Za-z0-9_-]+$");	//Reg-exp with allowed characters in XML tag's name

		if (tagName.length() == 0)
			return false;

		if (tagName.length() > 1000)
			return false;

		if (!std::regex_match(tagName, tagName_re))	//If name of tag not matches reg-exp
			return false;
		else
			return true;
	}

	//Transforms content from pure state to XML-presented and vice-versa
	//Casts some characters to their codes and codes to pure characters.
	void TransformContent(std::string& content, bool fromXMLText)	
	{
		//Needed lambdas for our routine
		auto ReplaceCharWithString = [](std::string& fullstr, size_t i, const std::string& str)
		{
			std::string s = fullstr.substr(0, i) + str + fullstr.substr(i + 1, (fullstr.length() - 1) - i);
			fullstr = s;
		};

		auto ReplaceStringWithChar = [](std::string& fullstr, size_t i, size_t count, char c)
		{
			std::string s = fullstr.substr(0, i);
			s.push_back(c);
			s += fullstr.substr(i + count, fullstr.length() - (i + count));
			fullstr = s;
		};

		std::string newContent;	//New value of content
		size_t first = 1, pos = 0, pos2 = 0;

		//All base of characters and their codes.
		//Stored in pairs <character, code>
		std::vector<std::pair<char, std::string>> xmlSymbols;
		xmlSymbols.push_back(std::pair<char, std::string>('>', "&gt;"));
		xmlSymbols.push_back(std::pair<char, std::string>('<', "&lt;"));
		xmlSymbols.push_back(std::pair<char, std::string>('&', "&amp;"));
		xmlSymbols.push_back(std::pair<char, std::string>('\'', "&apos;"));
		xmlSymbols.push_back(std::pair<char, std::string>('"', "&quot;"));

		if (!fromXMLText)	//If we transforming from XML text to pure
		{
			for (auto it = xmlSymbols.begin(); it != xmlSymbols.end(); it++)
			{
				first = 0;
				pos = 0;
				pos2 = 0;
				while (true)
				{
					pos = content.find(it->first, pos + 1 - first);
					pos2 = content.find(it->second, pos + 1 - first);
					first = 0;
					if (pos != std::string::npos && pos != pos2)
						ReplaceCharWithString(content, pos, it->second);
					else
						break;

				}
			}
		}
		else  //If we transforming from pure text to XML
		{
			for (auto it = xmlSymbols.begin(); it != xmlSymbols.end(); it++)
			{
				first = 0;
				pos = 0;
				while (true)
				{
					pos = content.find(it->second, pos + 1 - first);
					first = 0;
					if (pos != std::string::npos)
						ReplaceStringWithChar(content, pos, it->second.length(), it->first);
					else
						break;

				}
			}
		}
	}

	//Counts lines in string till specific position
	size_t CountLines(const std::string& s, size_t tillPosition)	
	{
		size_t count = 0;
		size_t pos = 0;
		while (pos < tillPosition)
		{
			pos = s.find('\n', pos + 1);
			count++;         
		}
		return count;
	}

	//Clean-up XML-file buffer from comments
	void RemoveComments(std::string& buffer)
	{
		size_t begin, end;
		std::string tmp;

		while (true)
		{
			begin = buffer.find("<!--", 0);	//Begin of comment
			if (begin == std::string::npos)	//When no comments left - break
				break;
			end = buffer.find("-->", begin);	//End of comment
			tmp = buffer.substr(0, begin);
			
			if(end != std::string::npos)	//If comment is not endless - add all content to temp buffer after and of comment
				tmp += buffer.substr(end + 3, (buffer.length() - (end + 3)));
			
			buffer = tmp; 
		}
	}
}

CXMLMgr::XML_Tree_Node::XML_Tree_Node()
{
	_tagName = "";
	_value = "";
	_top = nullptr;	//By default new node is root
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
	_UpdateTopNodes();	//Update top nodes of all child nodes when copy created
}

CXMLMgr::XML_Tree_Node& CXMLMgr::XML_Tree_Node::operator=(const CXMLMgr::XML_Tree_Node& copy)
{
	_tagName = copy._tagName;
	_value = copy._value;
	_top = copy._top;
	_nodes = copy._nodes;
	_it = _nodes.begin();
	_UpdateTopNodes();	//Update top nodes of all child nodes when copy created
	return *this;
}

std::string CXMLMgr::XML_Tree_Node::GetTagName() const
{
	return _tagName;
}

void CXMLMgr::XML_Tree_Node::SetTagName(std::string tagName)
{
	if(!XML_Utils::CheckTagName(tagName))	
		throw std::runtime_error("XML Parsing error: Bad tag name."); //Throw an error if name is bad
	_tagName = tagName;
}

void CXMLMgr::XML_Tree_Node::_UpdateTopNodes()
{
	for (size_t i = 0; i < _nodes.size(); i++)
	{
		for (size_t j = 0; j < _nodes[i]._nodes.size(); j++)
		{
			_nodes[i]._UpdateTopNodes();	//Recursively update child nodes of child nodes (Until we reach node without child nodes)
			_nodes[i]._nodes[j]._top = &_nodes[i];	//Assign valid value of top node pointer
		}
	}
}

bool CXMLMgr::XML_Tree_Node::Destroy()
{
	if (!_top)	//We cannot destroy root node
		return false;

	//Finds this node among child nodes of top node
	for (auto it = _top->_nodes.begin(); it != _top->_nodes.end(); it++)
	{
		if (&(*it) == this) //When node found
		{
			_top->_nodes.erase(it);	//Erase this node from container
			_top->_it = _top->_nodes.begin();	//Reset iterator of top node to begin (We updated it's container)
			_top->_UpdateTopNodes();	//Updating top node pointers
			break;
		}
	}
	return true;
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::GetLastNode()
{
	size_t size = _nodes.size();
	if (size == 0)	//Return nullptr if no child nodes
		return nullptr;
	else
		return &_nodes[size-1];	//Return last node
}

void CXMLMgr::XML_Tree_Node::RemoveNode(size_t i)
{
	_nodes.erase(_nodes.begin() + i);
	_it = _nodes.begin();
	_UpdateTopNodes();	//Update top node pointers of child after removing
}

void CXMLMgr::XML_Tree_Node::RemoveNode(std::string tagName, size_t skipMatches)
{
	size_t matches = 0;	//Match count
	for (auto it = _nodes.begin(); it != _nodes.end(); it++)
	{
		if ((*it)._tagName == tagName)	//If iterated tag name matches our tag name
		{
			if (matches == skipMatches)	//If we matching it N times
			{
				_nodes.erase(it);	//Removing
				_it = _nodes.begin();	//Reseting top iterator
				_UpdateTopNodes();	//Updating top node pointers
				break;
			}
			matches++;	//Increase matches
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
	if (tagName.length() == 0)	//We are not able to search for a nameless tag
		throw std::runtime_error("XML Error: Cannot search for nameless tag.");
	size_t matches = 0;	//Match count
	for (auto it = _nodes.begin(); it != _nodes.end(); it++)
	{
		if ((*it)._tagName == tagName)
		{
			if (matches == skipMatches)
			{
				return &(*it);	//Return node when found
				break;
			}
			matches++;
		}
	}

	//If we didnt found valid node and didnt returned it - throw an error.
	char errmsg[1200];
	sprintf_s(errmsg, 1200, "XML GetNode error: Tag \"%s\"[%u] does not exist.", tagName.c_str(), skipMatches);
	throw std::runtime_error(errmsg);
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::GetNode(std::string tagName, bool createIfNotExist)
{
	if (tagName.length() == 0) //We are not able to search for a nameless tag
		throw std::runtime_error("XML Error: Cannot search for nameless tag."); 
	for (auto it = _nodes.begin(); it != _nodes.end(); it++)
	{
		if ((*it)._tagName == tagName)
		{
			return &(*it);//Return node when found
			break;
		}
	}

	//We not found any node
	if (createIfNotExist) //If we allow to create new node it this situation...
		return InsertNode(tagName); //...than create new node with such name and return pointer to it.
	else 
	{
		//Or throw an error, we dont found any node with such name.
		char errmsg[1200];
		sprintf_s(errmsg, 1200, "XML GetNode error: Tag \"%s\" does not exist.", tagName.c_str());
		throw std::runtime_error(errmsg);
	}
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::IterateNodes(bool reset)
{
	if (reset) //If we want to reset now iterator equals begin.
		_it = _nodes.begin();

	if (_nodes.size() == 0)	//If no child nodes - return nullptr
		return nullptr;

	if (_it == _nodes.end()) //If we reached end - return nullptr
		return nullptr;

	CXMLMgr::XML_Tree_Node* node = &(*_it);	//Create pointer to node
	_it++;			//Than increment iterator
	return node;	//Return pointer
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::LastIteratedNode()
{
	if (_nodes.size() == 0) //If no child nodes - return nullptr
		return nullptr;
	else if (_it == _nodes.end())
		return GetLastNode(); //If we reached end - return  last node
	else if (_it == _nodes.begin())	//If we are on begin just return first node
		return &(*_it);
	else
		return &(*(_it - 1));	//Otherwise - return _it - 1 node (_it contains future value of iterated node)
}

std::string CXMLMgr::XML_Tree_Node::BuildXMLOutput()
{
	if (GetNodeCount() == 0)	//If no child nodes - return an empty string
		return "";

	auto originalTopNode = _top;	//Save current top node pointer
	_top = nullptr;					//Temporarily make current node root 
	CXMLMgr::XML_Tree_Node* tmpNode, *node = this;	//Pointer to loop node
	std::string xml, tmpContent;	//xml - is result of output, tmpContent is for tag content parsing
	size_t depth = 0;	//How deep we are now (How much tags opened now). For fancy tabulation.
	bool first = false;	//If first iteration was done

	while (true)
	{
		//End if we are on root node now, we done first iteration and node count in root node is 1
		//Or end of we are on root node now and last iterated node of root node is last node.
		if (node->GetTopNode() == nullptr && (node->GetNodeCount() == 1 && first || (node->GetNodeCount() > 1 && node->LastIteratedNode() == node->GetLastNode())))
			break;

		if(!first)	//We done first iteration
			first = true;

		if (node->GetNodeCount() > 0)	//If we have child nodes
		{
			tmpNode = node->IterateNodes();	//Get next node
			if (!tmpNode)	//If we reached last node
			{
				depth--; //We are moving upper
				for (size_t i = 0; i < depth; i++)	//Make tabulation
					xml += "\t";
				xml += XML_Utils::formTag(node->GetTagName(), true) + "\n";	//Close last opened tag
				node = node->GetTopNode();	//Go to top node
				continue;
			}
			else //If tmpNode exists
			{
				for (size_t i = 0; i < depth; i++) //Make tabulation
					xml += "\t";
				node = tmpNode; //Assign tmpNode to our loop node
				xml += XML_Utils::formTag(node->GetTagName(), false);	//Open new tag
				if (node->GetNodeCount() > 0) //Make new line if this node has child nodes
					xml += "\n";
				depth++; //We are moving deeper
			}
		}
		else //If we have no child nodes
		{
			tmpContent = node->GetContent(); //Get content of current tag
			XML_Utils::TransformContent(tmpContent, false); //Transform it to XML text format
			xml += tmpContent; //Add to output
			xml += XML_Utils::formTag(node->GetTagName(), true) + "\n"; //Close tag
			node = node->GetTopNode(); //We are moving back to top node
			depth--; //We are moving upper
		}
	}
	_top = originalTopNode; //Restore back current top node pointer of this node
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

bool CXMLMgr::XML_Tree_Node::HasContent() const
{
	if (_value.length() > 0)
		return true;
	else
		return false;
}

std::string CXMLMgr::XML_Tree_Node::GetContent() const
{
	return _value;
}

void CXMLMgr::XML_Tree_Node::SetContent(std::string content)
{
	_value = content;
}

CXMLMgr::XML_Tree_Node* CXMLMgr::XML_Tree_Node::InsertNode(std::string tagName, std::string content)
{
	if (tagName.length() == 0) //We cannot create a nameless tag
		throw std::runtime_error("XML Error: Cannot create a nameless tag.");
	//Push back to node container new tag with 'this' top pointer.
	_nodes.push_back(CXMLMgr::XML_Tree_Node(this, tagName, content));
	_it = _nodes.begin(); //Reset iterator to begin
	_UpdateTopNodes(); //Update top node pointers
	_value = ""; //Reset current content value, tags with another tags with-in cannot have any content.
	return GetLastNode(); //Return pointer to last created node
}

void CXMLMgr::XML_Tree_Node::RemoveAllNodes()
{
	_nodes.clear();
}

CXMLMgr::CXMLMgr() :
m_fileName("")
{
}

CXMLMgr::CXMLMgr(std::string fileName) :
	m_fileName(fileName) //Save last used filename
{
	ParseFile();	//Parsing file
}


void CXMLMgr::ParseFile()
{
	if (_access(m_fileName.c_str(), 0) == -1)	//Return if file does not exist
		return;

	//Reading file
	std::ifstream f(m_fileName);	
	std::string fileContent; //Buffer for file content
	f.seekg(0, std::ios::end);
	fileContent.reserve(size_t(f.tellg()));
	f.seekg(0, std::ios::beg);

	//Save entire file to string
	fileContent.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	f.close(); //Closing file

	size_t fileLen = fileContent.length(); //Length of file content

	if (fileLen == 0) //Return if file is empty
		return;

	XML_Utils::RemoveComments(fileContent); //Remove all XML-styled comments from file buffer
	std::string tagName, content; 
	std::stack<std::pair<std::string, size_t>> openedTags; //Stack of opened tags on loop iterations
	
	//dest - end of tag, offset - current offset to search, tempOffset - for finding closing tags, 
	size_t dest = 0, offset = 0, tempOffset = 0, contentLength = 0;
	bool closingTag = false;	//If we are on closing tag now
	CXMLMgr::XML_Tree_Node* node = &m_TreeRoot; //Current node (Starting from root)



	while(true)
	{
		offset = fileContent.find("<", offset);	//Find '<' starting by offset
		tempOffset = fileContent.find("</", offset);  //Find '</' starting by offset

		if (offset == std::string::npos && tempOffset == std::string::npos) //If no tags was found
		{
			if (openedTags.size() > 0)	//If we have unclosed tags when no more tags left in file
			{
				//Throwing an exeption with an additional info (unclosed tag and line of it)
				m_fileName = "";
				char errmsg[1200];
				size_t line = XML_Utils::CountLines(fileContent, openedTags.top().second);
				sprintf_s(errmsg, 1200, "XML Parsing error: Reached end of the file, but tag \"%s\"on line %u.", openedTags.top().first.c_str(), line);
				throw std::runtime_error(errmsg);
			}
			break; //Break loop if we parsed well and all tags is closed
		}
		else
		{
			if (offset == tempOffset) //If nearest tag is closing
			{
				closingTag = true; //Working with closing tags...
				offset++; //Increasing an offset
			}
			else
				closingTag = false; //Working with opening tags...

			dest = fileContent.find(">", offset); //Find end of tag
			if (dest == std::string::npos) //If tag does not have any end
			{
				//Throwing an exeption with an additional info (line of endless tag)
				m_fileName = "";
				char errmsg[128];
				size_t line = XML_Utils::CountLines(fileContent, offset);
				sprintf_s(errmsg, 128, "XML Parsing error: Endless tag found on line %u.", line);
				throw std::runtime_error(errmsg);
			}

			tagName = fileContent.substr(offset + 1, dest - (offset + 1)); //Save tag name
			if (tagName.length() == 0) //If tag is nameless
			{
				//Throwing an exeption with an additional info (line of nameless tag)
				m_fileName = "";
				char errmsg[128];
				size_t line = XML_Utils::CountLines(fileContent, offset);
				sprintf_s(errmsg, 128, "XML Parsing error: Nameless tag found on line %u.", line);
				throw std::runtime_error(errmsg);
			}

			if(tagName.length() > 1000) //If our tag is too long
			{
				//Throwing an exeption with an additional info (line of nameless tag)
				m_fileName = "";
				char errmsg[128];
				size_t line = XML_Utils::CountLines(fileContent, offset);
				sprintf_s(errmsg, 128, "XML Parsing error: Too long tag name on line %u.", line);
				throw std::runtime_error(errmsg);
			}

			if (!XML_Utils::CheckTagName(tagName)) //Check if our tag name is correct
			{
				//if not - throw an exeption with an additional info (name and line of tag)
				m_fileName = "";
				char errmsg[1200];
				size_t line = XML_Utils::CountLines(fileContent, offset);
				sprintf_s(errmsg, 1200, "XML Parsing error: Bad tag name \"%s\" on line %u.", tagName.c_str(), line);
				throw std::runtime_error(errmsg);
			}

			if (closingTag) //If current tag is closing
			{
				if (tagName == openedTags.top().first) //If closing tag matching last opened tag
				{
					contentLength = offset - openedTags.top().second - 1; //Calculate length of tag's content
					content = fileContent.substr(openedTags.top().second, contentLength); //Write
					XML_Utils::TransformContent(content, true); //Make content pure
					node->SetContent(content); //Save content to XML node
					if (node->GetTopNode()) //If we have top node - set it's content to an empty
						node->GetTopNode()->SetContent("");
					node = node->GetTopNode();  //Move to the top node
					openedTags.pop(); //Pop last opened tag from stack
				}
				else //If closing tag not matching last opened
				{
					//Throwing an exeption with an additional info (line of closing tag, name of closing tag, name of expected tag)
					m_fileName = ""; 
					size_t line = XML_Utils::CountLines(fileContent, offset);
					throw std::runtime_error(std::string(std::string("XML Parsing error: Unexpected closing tag \"</") + tagName + ">\" on line " + std::to_string(line) +  ", expected: \"</" + openedTags.top().first + ">\"").c_str());
				}
			}
			else //If tag is opening
			{
				openedTags.push(std::pair<std::string, size_t>(tagName, dest + 1)); //Push last opened tag to stack of opened tags
				node = node->InsertNode(tagName); //Create new node and move to created node
			}

			offset = dest; //Offset is end of the tag
		}
	}
}

void CXMLMgr::LoadFrom(std::string fileName)
{
	m_TreeRoot.RemoveAllNodes(); //Clear our tree root
	m_fileName = fileName; //Save last used filename
	ParseFile(); //Parsing a file
}

void CXMLMgr::Reload()
{
	m_TreeRoot.RemoveAllNodes(); //Clear our tree root
	ParseFile(); //Parse it again
}

void CXMLMgr::Save()
{
	std::ofstream f(m_fileName);
	f << m_TreeRoot.BuildXMLOutput(); //Build XML output and write it to the file
	f.close();
}

void CXMLMgr::SaveTo(std::string fileName)
{
	m_fileName = fileName; //Save last used filename
	Save();
}

CXMLMgr::XML_Tree_Node* CXMLMgr::GetRootNode()
{
	return &m_TreeRoot;	//Returns pointer to root node.
}