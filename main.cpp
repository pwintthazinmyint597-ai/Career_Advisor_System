#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
using namespace std;

#define LIGHT_PURPLE "\033[38;5;141m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define PINK    "\033[95m"   // Bright pink for text
#define BLUE    "\033[1;34m" // Bright blue for header
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

// ----------------- Career Struct -----------------
struct CareerInfo {
    string name;
    vector<int> prefs;
    vector<string> skills;
    vector<string> roadmap;
    vector<string> requiredSkills;
    vector<string> requiredEducation;
    int score=0;
};

// ----------------- Utility Functions -----------------
int askIntInRange(const string &prompt, int lo, int hi) {
    int v;
    while(true){
        cout << prompt;
        if(cin >> v && v>=lo && v<=hi){ cin.ignore(1024,'\n'); return v; }
        cout <<"Invalid input. Enter " << lo << "-" << hi << ".\n";

        cin.clear(); cin.ignore(1024,'\n');
    }
}

vector<int> askQuestions(const vector<string> &questions){
    vector<int> answers;
    for(size_t i=0;i<questions.size();++i){
        string p = to_string(i+1)+") "+questions[i]+" (1 = A little interested, 2 = Somewhat interested, 3 = Moderately interested, 4 = Very interested, 5 = Extremely interested, 0 = Back): ";
        int ans = askIntInRange(p,0,5);
        if(ans==0) return {};
        answers.push_back(ans);
    }
    return answers;
}

vector<string> askStringList(const string &prompt) {
    vector<string> items;
    cout << prompt << " (type 'done' to finish)\n";
    string s;
    while(true){
        cout << "> ";
        cin >> ws; // skip spaces
        getline(cin, s);
        if(s=="done") break;
        if(!s.empty()) items.push_back(s);
    }
    return items;
}

void computeScores(vector<CareerInfo> &careers,const vector<int> &answers){
    for(auto &c : careers){
        int sc=0;
        size_t n=min(c.prefs.size(),answers.size());
        for(size_t i=0;i<n;++i) sc+=c.prefs[i]*answers[i];
        c.score=sc;
    }
}

void adjustScoresWithBackground(vector<CareerInfo> &careers,
                                   const vector<string> &userSkills,
                                   const vector<string> &userEdu){
    for(auto &c : careers){
        for(auto &rs : c.requiredSkills){
            if(find(userSkills.begin(), userSkills.end(), rs)!=userSkills.end()){
                c.score += 5;
            }
        }
        for(auto &re : c.requiredEducation){
            if(find(userEdu.begin(), userEdu.end(), re)!=userEdu.end()){
                c.score += 10;
            }
        }
    }
}

// ----------------- Show Careers -----------------
void showCareerDetailed(const CareerInfo &c, int rank){
    cout << rank << ". " << c.name << " (Score: " << c.score << ")\n";
}

void showCareerDetails(const CareerInfo &c){
    cout << "\nCareer: " << c.name << "\n";
    cout << "  Skills Needed:\n";
    for(auto &s : c.skills) cout << "   - " << s << "\n";
    cout << "  Roadmap:\n";
    for(size_t i=0;i<c.roadmap.size(); ++i) cout << "   " << i+1 << ". " << c.roadmap[i] << "\n";
    cout << "\n";
}

void showTopCareers(vector<CareerInfo> &careers,int topN=3){
    sort(careers.begin(),careers.end(),[](const CareerInfo &a,const CareerInfo &b){ return a.score>b.score;});
    int n=min(topN,(int)careers.size());
    cout << "\n=== Top " << n << " career recommendations ===\n";
    for(int i=0;i<n;++i) showCareerDetailed(careers[i], i+1);

    cout << "\nDo you want to see skills & roadmap?\n";
    cout << "1. Show ALL Top " << n << " careers\n";
    cout << "2. Show detail for one career\n";
    cout << "0. Skip\n";
    int choice = askIntInRange("Choose option: ",0,2);

    if(choice==1){
        for(int i=0;i<n;++i) showCareerDetails(careers[i]);
    } else if(choice==2){
        int sel = askIntInRange("Enter career number (1-"+to_string(n)+"): ",1,n);
        showCareerDetails(careers[sel-1]);
    }
}

void saveTopCareersToFile(vector<CareerInfo> &careers,int topN=3){
    ofstream fout("TopCareers.txt");   // open file for writing
    if(!fout){ cout<<"Error opening file!\n"; return; }    // check if file failed to open
    sort(careers.begin(),careers.end(),[](const CareerInfo &a,const CareerInfo &b){ return a.score>b.score;});
    int n=min(topN,(int)careers.size());
    fout << "=== Top " << n << " career recommendations ===\n";
    for(int i=0;i<n;++i) fout << i+1 << ". " << careers[i].name << " (Score: " << careers[i].score << ")\n";
    fout.close();
    cout << "Top careers saved to TopCareers.txt\n";
}

void showSavedCareers(){
    ifstream fin("TopCareers.txt");  // open file for reading
    if(!fin){ cout<<"No saved file found.\n"; return; }
    string line;
    cout << PINK << "\n=== Saved Top Careers ===\n";
    while(getline(fin,line)) cout << line << "\n";
    fin.close();
}

// ----------------- Load from file -----------------
vector<string> loadQuestionsFromFile(const string &filename,const string &subfieldTag){
    ifstream fin(filename);
    if(!fin){ cout << "Cannot open " << filename << "\n"; return {}; }
    vector<string> questions;
    string line;
    bool reading=false;
    while(getline(fin,line)){
        if(line.empty()) continue;
        if(line[0]=='#'){ reading = (line==subfieldTag); continue; }
        if(reading) questions.push_back(line);
    }
    fin.close();
    return questions;
}

vector<CareerInfo> loadCareersFromFile(const string &filename,const string &subfieldTag){
    ifstream fin(filename);
    if(!fin){ cout<<"Cannot open career file "<<filename<<"\n"; return {}; }
    vector<CareerInfo> careers;
    string line;
    bool reading=false;
    while(getline(fin,line)){
        if(line.empty()) continue;
        if(line[0]=='#'){ reading = (line==subfieldTag); continue; }
        if(reading){
            size_t p1=line.find('|');
            size_t p2=line.find('|',p1+1);
            size_t p3=line.find('|',p2+1);
            if(p1==string::npos || p2==string::npos || p3==string::npos) continue;
            CareerInfo c;
            c.name = line.substr(0,p1);
            string prefsStr=line.substr(p1+1,p2-p1-1);
            string skillsStr=line.substr(p2+1,p3-p2-1);
            string roadmapStr=line.substr(p3+1);
            size_t start=0,pos;
            while((pos=prefsStr.find(',',start))!=string::npos){ c.prefs.push_back(stoi(prefsStr.substr(start,pos-start))); start=pos+1; }
            c.prefs.push_back(stoi(prefsStr.substr(start)));
            start=0;
            while((pos=skillsStr.find(',',start))!=string::npos){ c.skills.push_back(skillsStr.substr(start,pos-start)); start=pos+1; }
            c.skills.push_back(skillsStr.substr(start));
            start=0;
            while((pos=roadmapStr.find(',',start))!=string::npos){ c.roadmap.push_back(roadmapStr.substr(start,pos-start)); start=pos+1; }
            c.roadmap.push_back(roadmapStr.substr(start));
            careers.push_back(c);
        }
    }
    fin.close();
    return careers;
}

// ----------------- Field Flow -----------------
void fieldFlow(const string &careerFile, const string &questionFile, const string &subfieldTag) {
    vector<CareerInfo> careers = loadCareersFromFile(careerFile, subfieldTag);
    if(careers.empty()){ cout<<"No career data found for this subfield.\n"; return; }

    vector<string> questions = loadQuestionsFromFile(questionFile, subfieldTag);
    vector<int> answers;
    if(!questions.empty()) answers = askQuestions(questions);
    if(!answers.empty()) computeScores(careers, answers);

    showTopCareers(careers, 3);
    saveTopCareersToFile(careers,3);
}

// ----------------- Browse Mode -----------------
void browseCareerInfo() {
    while (true) {
        cout <<BLUE<< "\n================ Browse Skills & Roadmap ================\n";
        cout << "Choose Main Field:\n";
        cout << "1. IT\n2. Medicine\n3. Engineering\n4. Education\n5. Entertainment\n6. Art\n7. Economic\n8. Law\n9. Politics\n10. Other\n0. Back\n";
        int field = askIntInRange("Select field: ",0,10);
        if(field==0) return;

        string subfieldTag;
        int sub=0;

        if(field==1){ // IT
            cout << BLUE << "\n--- IT Subfields ---\n1. Hardware\n2. Software\n0. Back\n";
            sub = askIntInRange("Choose subfield: ",0,2);
            if(sub==0) continue;

            if(sub==1){
                cout << BLUE << "\nHardware Subfields:\n1. Computer Hardware Engineering\n2. Networking & Telecommunications\n3. Cloud Infrastructure & Virtualization\n4. IoT & Embedded Systems\n0. Back\n";
                int hwSub = askIntInRange("Choose subfield: ",0,4);
                if(hwSub==0) continue;
                subfieldTag = "#Hardware_Subfield_" + to_string(hwSub);
            }
            else { // Software
                cout << BLUE <<"\nSoftware Subfields:\n1. Programming & Software Development\n2. Web Technologies\n3. Database Systems & IS\n4. Cybersecurity\n5. Data Science & AI\n6. HCI & Design (UI/UX)\n0. Back\n";
                int swSub = askIntInRange("Choose subfield: ",0,6);
                if(swSub==0) continue;
                subfieldTag = "#Software_Subfield_" + to_string(swSub);
            }
        }
        else if(field==2){
            cout << BLUE <<"\nMedicine Subfields:\n1. General Medicine\n2. Surgery\n3. Pediatrics\n4. Medical Research\n0. Back\n";
            sub=askIntInRange("Choose subfield: ",0,4);
            if(sub==0) continue;
            subfieldTag="#Medicine_Subfield_"+to_string(sub);
        }
        else if(field==3){
            cout << BLUE << "\nEngineering Subfields:\n1. Mechanical Engineering\n2. Civil Engineering\n3. Electrical Engineering\n4. Computer Engineering\n0. Back\n";
            sub=askIntInRange("Choose subfield: ",0,4);
            if(sub==0) continue;
            subfieldTag="#Engineering_Subfield_"+to_string(sub);
        }
        else if(field==4){
            cout << BLUE <<"\nEducation Subfields:\n1. School Teacher\n2. University Lecturer\n3. Educational Technology Specialist\n4. Education Administration\n0. Back\n";
            sub=askIntInRange("Choose subfield: ",0,4);
            if(sub==0) continue;
            subfieldTag="#Education_Subfield_"+to_string(sub);
        }
        else if(field==5){
            cout << BLUE << "\nEntertainment Subfields:\n1. Actor / Actress\n2. Musician / Singer\n3. Dancer\n4. Filmmaker / Director\n5. Game & Media Production\n0. Back\n";
            sub=askIntInRange("Choose subfield: ",0,5);
            if(sub==0) continue;
            subfieldTag="#Entertainment_Subfield_"+to_string(sub);
        }
        else if(field==6){
            cout << BLUE << "\nArt Subfields:\n1. Painter\n2. Sculptor\n3. Graphic Designer\n4. Photographer\n0. Back\n";
            sub=askIntInRange("Choose subfield: ",0,4);
            if(sub==0) continue;
            subfieldTag="#Art_Subfield_"+to_string(sub);
        }
        else if(field==7){
            cout << BLUE <<"\nEconomic Subfields:\n1. Economist\n2. Financial Analyst\n3. Market Research Analyst\n4. Policy Analyst\n0. Back\n";
            sub=askIntInRange("Choose subfield: ",0,4);
            if(sub==0) continue;
            subfieldTag="#Economic_Subfield_"+to_string(sub);
        }
        else if(field==8){
            cout << BLUE << "\nLaw Subfields:\n1. Criminal Lawyer\n2. Corporate Lawyer\n3. Human Rights Lawyer\n4. Legal Advisor\n5. Environmental Lawyer\n6. Intellectual Property Lawyer\n0. Back\n";
            sub=askIntInRange("Choose subfield: ",0,6);
            if(sub==0) continue;
            subfieldTag="#Law_Subfield_"+to_string(sub);
        }
        else if(field==9){
            cout << BLUE <<"\nPolitics Subfields:\n1. Politician / Elected Official\n2. Policy Analyst\n3. Diplomat / Foreign Service\n4. Political Consultant\n5. Campaign Manager\n6. Public Affairs Specialist\n0. Back\n";
            sub=askIntInRange("Choose subfield: ",0,6);
            if(sub==0) continue;
            subfieldTag="#Politics_Subfield_"+to_string(sub);
        }
        else if(field==10){
            cout << BLUE <<"\nOther Fields Subfields:\n1. Sports / Fitness\n2. Journalism / Media\n3. Science / Research\n4. Hospitality / Tourism\n5. Environment / Agriculture\n0. Back\n";
            sub=askIntInRange("Choose subfield: ",0,5);
            if(sub==0) continue;
            subfieldTag="#Other_Subfield_"+to_string(sub);
        }
        else {
            cout<< RED <<"Field browsing not yet implemented for this option.\n";
            cout << RESET ;
            continue;
        }

        // Load careers for the chosen subfield
        vector<CareerInfo> careers = loadCareersFromFile("careers.txt", subfieldTag);
        if(careers.empty()){
            cout<< RED << "No career data found for this subfield.\n";
            cout<< RESET;
            continue;
        }

        while(true){
            cout << BLUE << "\nCareers available:\n";
            for(size_t i=0;i<careers.size();++i){
                cout << i+1 << ". " << careers[i].name << "\n";
            }
            cout << "0. Back\n";
            int csel = askIntInRange("Select career to see skills & roadmap: ",0,careers.size());
            if(csel==0) break;
            showCareerDetails(careers[csel-1]);
        }
    }
}

// ----------------- Main -----------------
int main(){
    cout << "=======================================================================================================\n";
    cout << BLUE;
    cout << "   _____          _____  ______ ______ _____               _______      _______  _____  ____  _____  \n";
    cout << "  / ____|   /\\   |  __ \\|  ____|  ____|  __ \\        /\\   |  __ \\ \\    / /_   _|/ ____|/ __ \\|  __ \\ \n";
    cout << " | |       /  \\  | |__) | |__  | |__  | |__) |      /  \\  | |  | \\ \\  / /  | | | (___ | |  | | |__) |\n";
    cout << " | |      / /\\ \\ |  _  /|  __| |  __| |  _  /      / /\\ \\ | |  | |\\ \\/ /   | |  \\___ \\| |  | |  _  / \n";
    cout << " | |____ / ____ \\| | \\ \\| |____| |____| | \\ \\     / ____ \\| |__| | \\  /   _| |_ ____) | |__| | | \\ \\ \n";
    cout << "  \\_____/_/    \\_\\_|  \\_\\______|______|_|  \\_\\   /_/    \\_\\_____/   \\/   |_____|_____/ \\____/|_|  \\_\\\n";
    cout << RESET;


    cout << "=======================================================================================================\n";

    cout << CYAN    << "                                        WELCOME TO "
         << MAGENTA << "CAREER ADVISOR"
         << CYAN    << " PROGRAM\n"
         << RESET;

    cout << CYAN << "------------------------------------------------------------------------------------------------------\n";
    cout << " This program will help you:\n";
    cout << "   1) Explore different career fields\n";
    cout << "   2) Match your skills & education\n";
    cout << "   3) Get a personalized career roadmap\n";
    cout << "------------------------------------------------------------------------------------------------------\n\n";

    while(true){
        cout << CYAN <<"\n     Main Menu\n========================================\n";
        cout << "1. Explore by answering questions\n";
        cout << "2. Browse careers (skills & roadmap only)\n";
        cout << "3. Match my skills & education\n";
        cout << "4. Show Saved Careers\n";
        cout << "5. Exit\n";
        cout << "========================================\n";

        int opt = askIntInRange("Select option: ",1,5);
        if(opt==5){ cout<<"Thank you for using Career Advisor Program.\n";
                    cout<<"\"Wishing you success on your journey ahead - may this career guidance be the first step \ntoward a bright and fulfilling future.\"\n";
                    cout<<"Goodbye!\n"; break; }

        if(opt==1){
            cout << LIGHT_PURPLE <<"\nSelect a Main Field to explore:\n";
            cout << "1. IT\n2. Medicine\n3. Engineering\n4. Education\n5. Entertainment\n6. Art\n7. Economic\n8. Law\n9. Politics\n10. Other\n0. Back\n";
            int field = askIntInRange("Choose field: ",0,10);
            if(field==0) continue;

            string subfieldTag;
            if(field==1){ // IT
            cout << LIGHT_PURPLE <<"\n--- IT Subfields ---\n1. Hardware\n2. Software\n0. Back\n";
            int branch = askIntInRange("Choose branch: ",0,2);
            if(branch==0) continue;

            if(branch==1){ // Hardware
                cout << LIGHT_PURPLE << "\nHardware Subfields:\n1. Computer Hardware Engineering\n2. Networking & Telecommunications\n3. Cloud Infrastructure & Virtualization\n4. IoT & Embedded Systems\n0. Back\n";
                int hwSub = askIntInRange("Choose subfield: ",0,4);
                if(hwSub==0) continue;
                subfieldTag = "#Hardware_Subfield_" + to_string(hwSub);
            } else { // Software
                cout << LIGHT_PURPLE <<"\nSoftware Subfields:\n1. Programming & Software Development\n2. Web Technologies\n3. Database Systems & IS\n4. Cybersecurity\n5. Data Science & AI\n6. HCI & Design (UI/UX)\n0. Back\n";
                int swSub = askIntInRange("Choose subfield: ",0,6);
                if(swSub==0) continue;
                subfieldTag = "#Software_Subfield_" + to_string(swSub);
                }
            } if(field==2){ // Medicine
                cout << LIGHT_PURPLE <<"\n--- Medicine Subfields ---\n1. General Medicine\n2. Surgery\n3. Pediatrics\n4. Medical Research\n0. Back\n";
                int sub = askIntInRange("Choose subfield: ",0,4);
                if(sub==0) continue;
            subfieldTag = "#Medicine_Subfield_" + to_string(sub);
            } else if(field==3){ // Engineering
                cout << LIGHT_PURPLE << "\n--- Engineering Subfields ---\n1. Mechanical Engineering\n2. Civil Engineering\n3. Electrical Engineering\n4. Computer Engineering\n0. Back\n";
                int sub = askIntInRange("Choose subfield: ",0,4);
                if(sub==0) continue;
                subfieldTag = "#Engineering_Subfield_" + to_string(sub);
            } else if(field==4){ // Education
                cout << LIGHT_PURPLE <<"\n--- Education Subfields ---\n1. School Teacher\n2. University Lecturer\n3. Educational Technology Specialist\n4. Education Administration\n0. Back\n";
                int sub = askIntInRange("Choose subfield: ",0,4);
                if(sub==0) continue;
                subfieldTag = "#Education_Subfield_" + to_string(sub);
            } else if(field==5){ // Entertainment
                cout << LIGHT_PURPLE << "\n--- Entertainment Subfields ---\n1. Actor / Actress\n2. Musician / Singer\n3. Dancer\n4. Filmmaker / Director\n5. Game & Media Production\n0. Back\n";
                int sub = askIntInRange("Choose subfield: ",0,5);
                if(sub==0) continue;
                subfieldTag = "#Entertainment_Subfield_" + to_string(sub);
            } else if(field==6){ // Art
                cout << LIGHT_PURPLE << "\n--- Art Subfields ---\n1. Painter\n2. Sculptor\n3. Graphic Designer\n4. Photographer\n0. Back\n";
                int sub = askIntInRange("Choose subfield: ",0,4);
                if(sub==0) continue;
                subfieldTag = "#Art_Subfield_" + to_string(sub);
            } else if(field==7){ // Economic
                cout << LIGHT_PURPLE << "\n--- Economic Subfields ---\n1. Economist\n2. Financial Analyst\n3. Market Research Analyst\n4. Policy Analyst\n0. Back\n";
                int sub = askIntInRange("Choose subfield: ",0,4);
                if(sub==0) continue;
                subfieldTag = "#Economic_Subfield_" + to_string(sub);
            } else if(field==8){ // Law
                cout << LIGHT_PURPLE <<"\n--- Law Subfields ---\n1. Criminal Lawyer\n2. Corporate Lawyer\n3. Human Rights Lawyer\n4. Legal Advisor\n5. Environmental Lawyer\n6. Intellectual Property Lawyer\n0. Back\n";
                int sub = askIntInRange("Choose subfield: ",0,6);
                if(sub==0) continue;
                subfieldTag = "#Law_Subfield_" + to_string(sub);
            } else if(field==9){ // Politics
                cout << LIGHT_PURPLE <<"\n--- Politics Subfields ---\n1. Politician / Elected Official\n2. Policy Analyst\n3. Diplomat / Foreign Service\n4. Political Consultant\n5. Campaign Manager\n6. Public Affairs Specialist\n0. Back\n";
                int sub = askIntInRange("Choose subfield: ",0,6);
                if(sub==0) continue;
                subfieldTag = "#Politics_Subfield_" + to_string(sub);
            } else if(field==10){ // Other
                cout << LIGHT_PURPLE <<"\n--- Other Fields Subfields ---\n1. Sports / Fitness\n2. Journalism / Media\n3. Science / Research\n4. Hospitality / Tourism\n5. Environment / Agriculture\n0. Back\n";
                int sub = askIntInRange("Choose subfield: ",0,5);
                if(sub==0) continue;
                subfieldTag = "#Other_Subfield_" + to_string(sub);
        }

        // finally call fieldFlow
        fieldFlow("careers.txt","questions.txt",subfieldTag);
    }
        else if(opt==2){
            browseCareerInfo();
        }

else if(opt==3){
    cout << YELLOW << "\n=== Match My Skills & Education ===\n";

    // Step 1: Prompt user for skills
    vector<string> userSkills;
    while(userSkills.empty()){
        userSkills = askStringList("Enter your skills");
        if(userSkills.empty()) cout << "You must enter at least one skill.\n";
        }

    // Step 2: Prompt user for education level (High School / Undergraduate / Graduate)
    vector<string> eduOptions = {"High School", "Undergraduate", "Graduate"};
    string userEduLevel;
    while(true){
        cout << YELLOW << "Enter your education level:\n";
        for(size_t i=0;i<eduOptions.size();++i) cout << i+1 << ". " << eduOptions[i] << "\n";
        cout << "> ";
        int e = askIntInRange("Choose option: ",1,(int)eduOptions.size());
        userEduLevel = eduOptions[e-1];
        if(!userEduLevel.empty()) break;
    }

    // Step 3: Normalize user input
    auto normalize = [](const string &s){
        string res = s;
        transform(res.begin(), res.end(), res.begin(), ::tolower);
        res.erase(remove_if(res.begin(), res.end(), ::isspace), res.end());
        return res;
    };
    vector<string> normSkills;
    for(auto &s : userSkills) normSkills.push_back(normalize(s));
    string normEduLevel = normalize(userEduLevel);

    // Step 4: Load all careers
    vector<string> allSubfieldTags = {
        "#Hardware_Subfield_1","#Hardware_Subfield_2","#Hardware_Subfield_3","#Hardware_Subfield_4",
        "#Software_Subfield_1","#Software_Subfield_2","#Software_Subfield_3","#Software_Subfield_4","#Software_Subfield_5","#Software_Subfield_6",
        "#Medicine_Subfield_1","#Medicine_Subfield_2","#Medicine_Subfield_3","#Medicine_Subfield_4",
        "#Engineering_Subfield_1","#Engineering_Subfield_2","#Engineering_Subfield_3","#Engineering_Subfield_4",
        "#Education_Subfield_1","#Education_Subfield_2","#Education_Subfield_3","#Education_Subfield_4",
        "#Entertainment_Subfield_1","#Entertainment_Subfield_2","#Entertainment_Subfield_3","#Entertainment_Subfield_4","#Entertainment_Subfield_5",
        "#Art_Subfield_1","#Art_Subfield_2","#Art_Subfield_3","#Art_Subfield_4",
        "#Economic_Subfield_1","#Economic_Subfield_2","#Economic_Subfield_3","#Economic_Subfield_4",
        "#Law_Subfield_1","#Law_Subfield_2","#Law_Subfield_3","#Law_Subfield_4","#Law_Subfield_5","#Law_Subfield_6",
        "#Politics_Subfield_1","#Politics_Subfield_2","#Politics_Subfield_3","#Politics_Subfield_4","#Politics_Subfield_5","#Politics_Subfield_6",
        "#Other_Subfield_1","#Other_Subfield_2","#Other_Subfield_3","#Other_Subfield_4","#Other_Subfield_5"
    };

    vector<CareerInfo> allCareers;
    for(auto &tag : allSubfieldTags){
        vector<CareerInfo> c = loadCareersFromFile("careers.txt", tag);
        allCareers.insert(allCareers.end(), c.begin(), c.end());
    }
    if(allCareers.empty()){
        cout << "No career data available.\n";
        continue;
    }

    // Step 5: Score calculation based on skills + education level
    for(auto &c : allCareers){
        int score = 0;

        // Skills
        vector<string> careerSkillsNorm;
        for(auto &s : c.skills) careerSkillsNorm.push_back(normalize(s));
        for(auto &rs : careerSkillsNorm){
            if(find(normSkills.begin(), normSkills.end(), rs) != normSkills.end()) score += 5;
        }

        // Education level (requiredEducation field)
        for(auto &re : c.requiredEducation){
            string reNorm = normalize(re);
            if(reNorm == normEduLevel) score += 10;
        }

        c.score = score;
    }

    // Step 6: Filter positive matches
    vector<CareerInfo> matchedCareers;
    for(auto &c : allCareers){
        if(c.score > 0) matchedCareers.push_back(c);
    }

    if(matchedCareers.empty()){
        cout << RED <<"\nNo matching career found for your skills and education level.\n";
    } else {
        showTopCareers(matchedCareers, 3);
    }
}
    else if(opt==4){
            showSavedCareers();
        }
    }

    return 0;
}

