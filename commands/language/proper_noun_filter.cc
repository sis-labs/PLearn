#include <plearn/base/general.h>
//#include "GrowingStringTable.h"
#include <plearn/base/stringutils.h>        //!< For pgetline.
#include <plearn_learners/language/WordNet/WordNetOntology.h>

using namespace PLearn;

// usage : proper_noun_filter proper_nouns stop_words < in.text > filtered_out.text

set<string> extractWordSet(string file)
{
    set<string> words;
    ifstream in(file.c_str());
    while (!in.eof())
    {
        string line = pgetline(in);
        if (line == "") continue;
        words.insert(line);
    }
    in.close();
    return words;
}

int main(int argc, char** argv)
{
    if (argc != 3)
        PLERROR("usage : proper_noun_filter proper_nouns stop_words < in.text > filtered_out.text");
    //GrowingStringTable proper_nouns(argv[1]);
    //GrowingStringTable stop_words(argv[2]);
    set<string> proper_nouns = extractWordSet(argv[1]);
    set<string> stop_words = extractWordSet(argv[2]);
    WordNetOntology wn;
    string word;
    while (true)
    {
        cin >> word;
        if (!cin) break;
        word = lowerstring(word);
        //unsigned int pn_id = proper_nouns.elementNumber(word.c_str());
        //unsigned int sw_id = stop_words.elementNumber(word.c_str());
        bool is_proper_noun = proper_nouns.find(word) != proper_nouns.end();
        bool is_stop_word = stop_words.find(word) != stop_words.end();
        if (is_proper_noun && !is_stop_word && !wn.isInWordNet(word))
            cout << "<proper_noun>" << endl;
        else
            cout << word << endl;
    }
}


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
