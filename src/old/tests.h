#ifndef __TESTS_H__
#define __TESTS_H__
#include <vector>

void 		runTests			(void);
void		testRename			(std::vector<class Scene> &);
void	    testDates			(std::string s);
void        testTokenizer		(std::string testString, std::string tokenString);
void        testRTokenizer		(std::string testString, std::string tokenList);
void        testParseCast		(std::string testString);
void		testParsingFunctions(std::string testString);
void        testParseTitle		(std::string testString);
void        testStrRem			(std::string &testString, char, bool);
void 		testIAFDquery		(std::string name);
void 		testImageDownload	(std::string name);
void 		testAgeCalculator	(class Actor a, class Scene s);
std::vector<class Filepath> 	readFilelistFromText(std::string path);
void testHeightAndWeight (std::string name);

#endif
