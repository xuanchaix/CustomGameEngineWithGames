#include <stdio.h>
#include <stdlib.h>
#include "GameCommon.hpp"
#include <vector>
#include <iostream>

void printStrings( Strings& s ) {
	printf( "split result:\n" );
	for (auto& str : s) {
		printf( "\"%s\" ", str.c_str() );
	}
	printf( "\n" );
}

InputSystem* g_theInput = nullptr;

class BaseThing {
public:
	virtual ~BaseThing() = 0 {};

	virtual bool SelfAdd( EventArgs& args ) = 0;
	virtual bool Response( EventArgs& args ) = 0;
};

class Thing : public BaseThing{
public:
	Thing() = default;
	
	bool SelfAdd( EventArgs& args );
	bool Response( EventArgs& args );


	int m_num1 = 1;
	int m_num2 = 2;
};

bool Thing::SelfAdd( EventArgs& args )
{
	UNUSED( args );
	int numAdd = m_num1 + m_num2;
	m_num1 = m_num2;
	m_num2 = numAdd;
	return true;
}

bool Thing::Response( EventArgs& args )
{
	printf( "%s\n", args.GetValue( "Data", "Nothing" ).c_str() );
	return true;
}

int a, b, c = 0;

bool Command_SetReg( EventArgs& args ) {
	a = atoi( args.GetValue( "a", "0" ).c_str() );
	b = atoi( args.GetValue( "b", "0" ).c_str() );
	c = atoi( args.GetValue( "c", "0" ).c_str() );
	return true;
}

bool Command_PrintReg( EventArgs& args ) {
	unsigned char reg = args.GetValue( "reg", "a" )[0];
	if (reg == 'a') {
		printf( "%d\n", a );
	}
	else if (reg == 'b') {
		printf( "%d\n", b );
	}
	else if (reg == 'c') {
		printf( "%d\n", c );
	}
	return true;
}

bool Command_ADD( EventArgs& args ) {
	unsigned char dst = args.GetValue( "dst", "a" )[0];
	unsigned char src1 = args.GetValue( "src1", "a" )[0];
	unsigned char src2 = args.GetValue( "src2", "a" )[0];
	int* ptrToDst = nullptr;
	if (dst == 'a') {
		ptrToDst = &a;
	}
	else if (dst == 'b') {
		ptrToDst = &b;
	}
	else if (dst == 'c') {
		ptrToDst = &c;
	}
	int* ptrToSrc1 = nullptr;
	if (src1 == 'a') {
		ptrToSrc1 = &a;
	}
	else if (src1 == 'b') {
		ptrToSrc1 = &b;
	}
	else if (src1 == 'c') {
		ptrToSrc1 = &c;
	}
	int* ptrToSrc2 = nullptr;
	if (src2 == 'a') {
		ptrToSrc2 = &a;
	}
	else if (src2 == 'b') {
		ptrToSrc2 = &b;
	}
	else if (src2 == 'c') {
		ptrToSrc2 = &c;
	}
	if (ptrToDst && ptrToSrc1 && ptrToSrc2) {
		*ptrToDst = *ptrToSrc1 + *ptrToSrc2;
	}
	return true;
}
// 
// #include <vector>
// #include <deque>
// 
// struct TreeNode
// {
// 	int value = 0;
// 	TreeNode* left = nullptr;
// 	TreeNode* right = nullptr;
// 	TreeNode() : value( 0 ), left( nullptr ), right( nullptr ) {}
// 	TreeNode( int x ) : value( x ), left( nullptr ), right( nullptr ) {}
// 
// 	// Add the code for this function and any supporting functions
// 	static TreeNode* FromSortedArray( std::vector<int>& values );
// 	static void PrintTree( TreeNode* root );
// protected:
// 	static void PopulateTreeFromSortedArray( TreeNode* root, int arrayStartIndex, int arrayEndIndex, std::vector<int>& values );
// };
// 
// TreeNode* TreeNode::FromSortedArray( std::vector<int>& values )
// {
// 	if (values.size() == 0) {
// 		return nullptr;
// 	}
// 	TreeNode* retValue = new TreeNode();
// 	PopulateTreeFromSortedArray( retValue, 0, (int)values.size() - 1, values );
// 	return retValue;
// }
// 
// void TreeNode::PopulateTreeFromSortedArray( TreeNode* root, int arrayStartIndex, int arrayEndIndex, std::vector<int>& values )
// {
// 	int middleIndex = (arrayStartIndex + arrayEndIndex) / 2;
// 	root->value = values[middleIndex];
// 	if (arrayStartIndex == arrayEndIndex) {
// 		return;
// 	}
// 	if (arrayStartIndex <= middleIndex - 1) {
// 		TreeNode* leftChild = new TreeNode();
// 		root->left = leftChild;
// 		PopulateTreeFromSortedArray( leftChild, arrayStartIndex, middleIndex - 1, values );
// 	}
// 	if (middleIndex + 1 <= arrayEndIndex) {
// 		TreeNode* rightChild = new TreeNode();
// 		root->right = rightChild;
// 		PopulateTreeFromSortedArray( rightChild, middleIndex + 1, arrayEndIndex, values );
// 	}
// }
// 
// void TreeNode::PrintTree( TreeNode* root )
// {
// 	std::deque<TreeNode*> queue;
// 	queue.push_back( root );
// 	bool isOnLastLevel = false;
// 	while (!queue.empty()) {
// 		TreeNode* thisNode = queue.front();
// 		queue.pop_front();
// 		if (thisNode) {
// 			printf( "%d ", thisNode->value );
// 			if (!isOnLastLevel) {
// 				queue.push_back( thisNode->left );
// 				queue.push_back( thisNode->right );
// 			}
// 		}
// 		else {
// 			isOnLastLevel = true;
// 			printf( "x " );
// 			continue;
// 		}
// 	}
// 	printf( "\n" );
// }
// 
// int main()
// {
// 	std::vector<int> sortedArray1 = { -10, -3, 0, 5, 9 };
// 	TreeNode* root1 = TreeNode::FromSortedArray( sortedArray1 );
// 	TreeNode::PrintTree( root1 );
// 	
// 	std::vector<int> sortedArray2 = { 1, 3 };
// 	TreeNode* root2 = TreeNode::FromSortedArray( sortedArray2 );
// 	TreeNode::PrintTree( root2 );
// 
// 	std::vector<int> sortedArray3 = { 0 };
// 	TreeNode* root3 = TreeNode::FromSortedArray( sortedArray3 );
// 	TreeNode::PrintTree( root3 );
// 
// 	std::vector<int> sortedArray4;
// 	TreeNode* root4 = TreeNode::FromSortedArray( sortedArray4 );
// 	TreeNode::PrintTree( root4 );
// 
// 
// 	std::vector<int> sortedArray5 = { -10, -9, -8, -7, -6, -5, -4, -3, -2, 0, 2, 4, 6, 8, 7, 9 };
// 	TreeNode* root5 = TreeNode::FromSortedArray( sortedArray5 );
// 	TreeNode::PrintTree( root5 );
// }


int main( int, char** )
{
	EventSystemConfig eConfig;
	g_theEventSystem = new EventSystem( eConfig );

	DevConsoleConfig dConfig;
	DevConsole devConsole( dConfig );
	g_devConsole = &devConsole;

	SubscribeEventCallbackFunction( "Command_Set", Command_SetReg );
	SubscribeEventCallbackFunction( "Command_Print", Command_PrintReg );
	SubscribeEventCallbackFunction( "Command_Add", Command_ADD );

	// NamedProperties: example usage
	std::string lastName( "Eiserloh" );
	NamedProperties employmentInfoProperties;
	// ...

	NamedProperties p;
	p.SetValue( "Height", 1.93f );
	p.SetValue( "Age", 50 );
	p.SetValue( "IsMarried", true );
	p.SetValue( "Position", Vec2( 3.5f, 6.2f ) );
	p.SetValue( "EyeColor", Rgba8( 77, 38, 23 ) );
	p.SetValue( "LastName", lastName );	// Set as std::string data...
	p.SetValue( "FirstName", "Squirrel" ); 	// Set as c-string (char const*)? Store as std::string!	
	p.SetValue( "EmployeeInfo", employmentInfoProperties ); // NamedProperties inside NamedProperties!

	float height = p.GetValue( "Height", 1.75f );
	int health = p.GetValue( "Health", 100 ); // Returns 100 if “Health” was not present
	std::string m_firstName = "AAA";
	m_firstName = p.GetValue( "FirstName", m_firstName ); // Variable as its own default value
	std::string m_lastName = "BBB";
	m_lastName = p.GetValue( "LastName", m_lastName );   // This is a common trick
	int height2 = p.GetValue( "Height", 76 ); // ERROR: Incorrect type!  Data value is float!

	NamedProperties employerInfo = p.GetValue( "EmployeeInfo", NamedProperties() ); // Nested!
	std::string ssn = employerInfo.GetValue( "SSN", "00000000" );

	// Note the subtleties in the Set, Get, and return types for each of the following examples:
	std::string unknownNameString = "UNKNOWN NAME";
	std::string lastName2 = p.GetValue( "LastName", unknownNameString );
	std::string lastName3 = p.GetValue( "LastName", "UNKNOWN" ); // Explicit override of GetValue!
	std::string firstName2 = p.GetValue( "FirstName", unknownNameString );
	std::string firstName3 = p.GetValue( "FirstName", "UNKNOWN" ); // Explicit override of SetValue!

	Thing thing;

	SubscribeEventCallbackFunction( "Response", &thing, &Thing::Response );
	SubscribeEventCallbackFunction( "Fib", (Thing*)&thing, &Thing::SelfAdd);

	EventArgs args;
	args.SetValue( "Data", Stringf( "%d", thing.m_num1 ) );
	FireEvent( "Response", args );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args  );

	EventArgs argsN;
	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args  );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args  );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args  );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args  );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args  );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args  );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args  );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args  );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args );

	UnsubscribeEventCallbackFunction( "Fib", &thing, &Thing::SelfAdd);

	printf( "Unsubscribe!\n" );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args );

	SubscribeEventCallbackFunction( "Fib", &thing, &Thing::SelfAdd );

	printf( "Subscribe!\n" );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args );

	UnsubscribeAllEventCallbackFunctionForObject( &thing );
	printf( "Unsubscribe All!\n" );

	FireEvent( "Fib", argsN );
	args.SetValue( "Data", Stringf( "%d", thing.m_num2 ) );
	FireEvent( "Response", args );

	Strings strs;
	SplitStringWithQuotes( strs, "CommandName a=b c=\"d\" e=\"f g\" h=\"i j=k l=m n=o\"", ' ' );

	devConsole.ExecuteXmlCommandScriptFile( "Test/TestCalculator.xml" );

	UNUSED( height2 );
	UNUSED( height );
	UNUSED( health );



	system( "pause" );
	return 0;
}
