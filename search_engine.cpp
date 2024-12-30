#include <iostream>
#include <fstream>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
using namespace std;



//search_engine.cpp by William Vu



class DataBase{
public:
    //file containing typical google searches. one search per line
    ifstream questions;
    //file containing links to websites answering those searches.
    ifstream answers;
    //file containing the title of the websites contained in the answers document
    ifstream answerTitles;
    
    
    //vector to store each line from the questions file in a string element
    vector<string> qArr;
    //vector to store each line from the answers file in a string element
    vector<string> aArr;
    //vector to store each line from the answersTitles file in a string element
    vector<string> aTitleArr;
    
    //Constructor
    DataBase();
} database;//database object

//turns a string into all lowercase
void turnLowercase(string& str){
    for(int i=0;i<str.length();++i){
        if(isupper(str[i])){
            str[i] = tolower(str[i]);
        }
    }
}

DataBase::DataBase(){
    
    //file containing 100 various google searches, each seperated by a newline
    questions.open("/Users/williamvu/Downloads/questions.txt");
    //file containing website links that answer each of the searches in questions.txt
    answers.open("/Users/williamvu/Downloads/answers.txt");
    //file containing the titles of each website in answers.txt
    answerTitles.open("/Users/williamvu/Downloads/answer_titles.txt");
    
    //check if the files opened properly
    //exit program if they fail to open
    if(questions.fail()){
        cerr << "Error opening questions file. Exiting program" << endl;
        exit(1);
    }
    if(answers.fail()){
        cerr << "Error opening answers file. Exiting program" << endl;
        exit(1);
    }
    if(answerTitles.fail()){
        cerr << "Error opening answer titles file. Exiting program" << endl;
        exit(1);
    }
    
    //add each line in the questions file into a vector
    string input;
    while(getline(questions, input)){
        qArr.push_back(input);
    }
    questions.close();
    
    //add each line in the answers file into a vector
    while(getline(answers, input)){
        aArr.push_back(input);
    }
    answers.close();
    
    //add each line in the answersTitles file into a vector
    while(getline(answerTitles, input)){
        aTitleArr.push_back(input);
    }
    answerTitles.close();
    
    //make all strings in qArr, aArr fully lowercase
    //to make the search algorithm simpler
    for(int i=0;i<qArr.size();++i){
        turnLowercase(qArr[i]);
    }
    for(int i=0;i<aArr.size();++i){
        turnLowercase(aArr[i]);
    }
    
    //Check if each vector has the same size
    //If not, resize every vector to the size of the smallest vector
    if(qArr.size() != aArr.size()  ||  qArr.size() != aTitleArr.size()){
        //find the smallest sized vector
        unsigned long smallest = qArr.size();
        if(aArr.size() < smallest){
            smallest = aArr.size();
        }
        if(aTitleArr.size() < smallest){
            smallest = aTitleArr.size();
        }
        cout << "Resizing the database to " << smallest << " available search results" << endl;
        qArr.resize(smallest);
        aArr.resize(smallest);
        aTitleArr.resize(smallest);
    }
    
}

//class to be returned by the "search" function
class SearchResult{
private:
    //array of integers that correspond to
    //the indexes of the aArr array (refer to the database class)
    vector<int> linkIndexes;
public:
    //Accessor function
    int getLinkIndex(int index);
    //returns size of linkIndexes
    int getSize();
    //Mutator function
    void setLinkIndex(int index, int newValue, vector<int> aArr);
    
    void addLinkIndex(int index);
};

int SearchResult::getLinkIndex(int index){
    if(index >= linkIndexes.size() || index < 0){
        cerr << "Error in getLinkIndex function: value passed invalid. Exiting program" << endl;
        exit(1);
    }
    return linkIndexes[index];
}

int SearchResult::getSize(){
    return static_cast<int>( linkIndexes.size() );
}

void SearchResult::setLinkIndex(int index, int newValue, vector<int> aArr){
    if(index >= linkIndexes.size() || index < 0){
        cerr << "Error in setLinkIndex function: value passed invalid. Exiting program" << endl;
        exit(1);
    }
    if(newValue >= aArr.size() || newValue < 0){
        cerr << "Error in setLinkIndex function: value passed invalid. Exiting program" << endl;
        exit(1);
    }
    linkIndexes[index] = newValue;
}

void SearchResult::addLinkIndex(int index){
    linkIndexes.push_back(index);
}

//given a sentence, counts how many times each individual word is used
//values in the hash map reflect the counts
//ignores punctuation besides ' (ASCII 39) asterisk
void countWordsInSentence(string sentence, unordered_map<string,int>& mp){
    string word;
    istringstream Sentence(sentence);
    while(Sentence >> word){
        //remove unnecessary punctuation at the end of the word
        //since punctuation never precedes a word
        //apostrophe is ignored
        if(!isalpha(word[word.length()-1])  &&  !isdigit(word[word.length()-1])  &&  word.length() > 1  &&  word[word.length()-1] != '\''){
            word.pop_back();
        }
        mp[word]++;
    }
}
//determines which of the questions in the database match with the user-entered question
//returns an object of the SearchResult class
SearchResult search(string question, vector<string> qArr, vector<string> aArr){
    //the object to be returned
    SearchResult result;
    //parallel array to qArr
    //the i'th element of qArr corresponds to the i'th element of scores
    // 0 <= score[i] <= 1
    vector<float> scores(qArr.size());
    
    //counts the occurence of each word in the "questions" string
    unordered_map <string,int> entryMap;
    turnLowercase(question);
    countWordsInSentence(question , entryMap);
    
    for(int i=0;i<qArr.size();++i){
        unordered_map<string,int> questionMap;
        questionMap = entryMap;
        unordered_map <string,int> qArrMap;
        countWordsInSentence(qArr[i] , qArrMap);
        int denom = static_cast<int>(  min(entryMap.size() , qArrMap.size())  );
        int numer = 0;//how many words in qArr[i] are in question
        //iterate through qArrMap
        for(auto& it : qArrMap){
            if(entryMap.find(it.first) != questionMap.end()){
                do{
                    ++numer;
                    it.second--;
                    questionMap[it.first]--;
                }while(it.second > 0  &&  questionMap[it.first] > 0);
            }
        }
        scores[i] = static_cast<float>(numer) / denom;
    }
    
    //get the 3 largest elements' indeces in the scores array
    int largest, seclargest, thirdlargest;
    largest = seclargest = thirdlargest = 0;
    for(int i=0;i<scores.size();++i){
        if(scores[i] > largest){
            thirdlargest = seclargest;
            seclargest = largest;
            largest = i;
        }
        else if(scores[i] > seclargest){
            thirdlargest = seclargest;
            seclargest = i;
        }
        else if(scores[i] > thirdlargest){
            thirdlargest = i;
        }
    }
    
    //check if largest, seclargest, or thirdlargest are the same indexes
    if(seclargest == largest){
        seclargest = -1;
    }
    if(seclargest == thirdlargest){
        thirdlargest = -1;
    }
    
    if(scores[largest] > 0.0)
        result.addLinkIndex(largest);
    if(scores[seclargest] > 0.0)
        result.addLinkIndex(seclargest);
    if(scores[thirdlargest] > 0.0)
        result.addLinkIndex(thirdlargest);
    
    return result;
    
}//end of search function

//opens a URL on the default browser
void openURL(string url){
    string command = "open " + url;
    system(command.c_str());
}

string cleanString(const string& str){
    //Function Actions:
    //clear all leading and trailing spaces
    //clear double spaces between words
    
    int firstNonSpace=-1;
    for(int i=0;i<str.length();++i){
        if(str[i] != ' '){
            firstNonSpace = i;
            break;
        }
    } if(firstNonSpace == -1) return string("");

    int lastNonSpace=-1;
    for(int i=str.length()-1;i>=0;--i){
        if(str[i] != ' '){
            lastNonSpace = i;
            break;
        }
    } if(lastNonSpace == -1) return string("");
    
    string newStr;

    for(int i=firstNonSpace;i<=lastNonSpace;++i){
        if(str[i] == ' '){
            if(newStr[newStr.length()-1] != ' '){
                newStr += ' ';
            }
        }
        else{
            newStr += str[i];
        }
    }

    return newStr;
}

//Driver Code
int main(){
    
    string searchEntry;
    while(1){
        if(searchEntry.length() > 0){
            searchEntry.clear();
        }
        cout << "To end program, enter \"DONE\"" << endl;
        cout << "Enter your search: ";
        char c;
        while(cin.get(c)){
            if(c == '\n'  &&  searchEntry.length() > 0) break;
            searchEntry += c;
        }
        
        if(searchEntry == "DONE"){
            break; //exit from the loop
        }
        
        SearchResult result = search(searchEntry, database.qArr, database.aArr);
        
        cout << "\n\n";
        for(int i=0;i<30;++i)
            cout << '-';
        cout << endl << "Results: \n\n";
        
        if(result.getSize() == 0){
            cout << "Your search: " << searchEntry << " did not match any documents" << endl;
        }
        else{
            //print the titles of each website in the search result
            for(int i=0;i<result.getSize();++i){
                cout << i+1 << "." << endl;
                cout << database.aTitleArr[ result.getLinkIndex(i) ] << endl;
            }
            
            //the user selects which links they want to open
            int linkNumber;
            do{
                cout << "\nTo exit selection, type -1" << endl;
                cout << "Enter which link you wish to open: ";
                cin >> linkNumber;
                if((linkNumber <= 0  ||  linkNumber > result.getSize()+1)  && linkNumber != -1){
                    cerr << "INVALID LINK NUMBER. TRY AGAGIN" << endl;
                    continue;
                }
                if(linkNumber == -1){
                    break;
                }
                cout << "Opening link #" << linkNumber << endl;
                openURL(database.aArr[ result.getLinkIndex(linkNumber-1) ]);
            }while(linkNumber != -1);
        }
        
        cout << "\n\n\n";
    }//loop end
    
    cout << endl << "Program Finished." << endl;
    return 0;
}
