#include<iostream>
#include<string>
#include<sstream>

#include<fstream>
#include<vector>
#include<algorithm>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace std;

class event
{
    int eventcount=0;
    protected: 
        int id;
        string name;
        string category;
        string date;
        string time;
        string venue;
        string organizer;
        string registration_link;

    public:
    event()
    {
        id = 0;
        eventcount = 0;
    }
    event(int &id, string &name, string &category, string &date, string &time, string &venue, string &organizer, string &registration_link)
    {
        this->id = id;
        this->name = name;
        this->category = category;
        this->date = date;
        this->time = time;
        this->venue = venue;
        this->organizer = organizer;
        this->registration_link = registration_link;
    }

    // FUNCTIONS PERFORMED 
    void get_data()
    {
        cout<<"Enter event id: ";
        cin>>id;
        cin.ignore();
        cout<<"Enter event name: ";
        getline(cin,name);
        cout<<"Enter category: ";
        getline(cin,category);
        cout<<"Enter date in format (date:Month:Year): ";
        getline(cin,date);
        cout<<"Enter time in format (Hour:Min:Sec): ";
        getline(cin,time);
        cout<<"Enter venue: ";
        getline(cin,venue);
        cout<<"Enter organizer: ";
        getline(cin,organizer);
        cout<<"Enter registration link: ";
        getline(cin,registration_link);
    }

    // GETTERS
    int get_id() const { return id; }
    string get_name() const { return name; }
    string get_category() const { return category; }
    string get_date() const { return date; }
    string get_time() const { return time; }
    string get_venue() const { return venue; }
    string get_organizer() const { return organizer; }
    string get_registration_link() const { return registration_link; }

    // FOR DISPLAYING EVENTS 
    void display_event() const
    {
        cout<<"------------------------------------------------------------------------------"<<endl;
        cout<<"Event id                     : "<<id<<endl;
        cout<<"Event name                   : "<<name<<endl;
        cout<<"Event category               : "<<category<<endl;
        cout<<"Event date                   : "<<date<<endl;
        cout<<"Event time                   : "<<time<<endl;
        cout<<"Event venue                  : "<<venue<<endl;
        cout<<"Event organizer              : "<<organizer<<endl;
        cout<<"Event registration link      : "<<registration_link<<endl;
        cout<<"-----------------------------------------------------------------------------"<<endl;
    }

    // CONVERTS DATA TO STRING FORMAT FOR BROWSER LOCALSTORAGE
    string to_filestring() const
    {  
        return to_string(id) + "|" + name  + "|" + category + "|" + date + "|" + time + "|" + venue + "|" + organizer + "|" + registration_link;
    }
};

class node
{
    public:
        event data;
        node* left;
        node* right;

        node(event ev)
        {
            data = ev;
            left = NULL;
            right = NULL;
        }
};

class bst : public event
{
    private:
        node* root = nullptr;
        string outBuf;

        void destroy(node *n)
        {
            if (n == NULL) return;
            destroy(n->left);
            destroy(n->right);
            delete n;
        }

    public:
    ~bst() { destroy(root); root = NULL; }
    void clear_tree() { destroy(root); root = NULL; }
    node*& get_root() { return root; }

    void collect_events(node* r, vector<event>& vec) const
    {
        if(r == NULL) return;
        collect_events(r->left, vec);
        vec.push_back(r->data);
        collect_events(r->right, vec);
    }   

    // INSERTING DATA IN BST
    bool insert(node*& r, event ev)
    {
        if(r == NULL){
            r = new node(ev);
            return true;
        }

        if(ev.get_id() < r->data.get_id())
        {
            return insert(r->left , ev);
        }
        else if(ev.get_id() > r->data.get_id())
            return insert(r->right , ev);
        else
        {
            cout<<"EVENT ALREADY EXISTS"<<endl;
            return false;
        }
    }

    // ADDS EVENTS
    void add_event(node*& r)
    {
        event ev;
        ev.get_data();
        bool inserted = insert(r, ev);

        if(inserted){
            cout<<"Event ADDED SUCCESSFULLY"<<endl;
        }
        else{
            cout<<"Event not added : Event already exists"<<endl;
        }
    }

    // SEARCH EVENT IN BST
    void search_event(node*& r, int id) const
    {
        if(r == NULL)
        {
            cout<<"Event not found"<<endl;
            return;
        }

        if(r->data.get_id() == id)
        {
            cout<<"Event found"<<endl;
            r->data.display_event();
            return;
        }
        else if(id < r->data.get_id())
        {
            search_event(r->left , id);
        }
        else 
        {
            search_event(r->right , id);
        }
    }

    // INORDER TRAVERSAL IN BST
    void inorder(node*& r) const
    {
        if(r == NULL)
            return;

        inorder(r->left);
        r->data.display_event();
        inorder(r->right);
    }

    // DELETE EVENT FROM BST
    bool delete_event(node*& r, int id)
    {
        if(r == NULL)
        {
            return false;
        }

        if(id < r->data.get_id())
            return delete_event(r->left , id);
        else if(id > r->data.get_id())
            return delete_event(r->right ,id);
        else 
        {
            if(r->left == NULL && r->right == NULL)
            {
                delete r;
                r = NULL;
            }
            else if(r->left == NULL)
            {
                node* temp = r;
                r = r->right;
                delete temp;
            }
            else if(r->right == NULL)
            {
                node* temp = r;
                r = r->left;
                delete temp;
            }
            else
            {
                node* parent = r->right;
                while(parent->left != NULL)
                    parent = parent->left;

                event tempEv = parent->data;
                delete_event(r->right , parent->data.get_id());
                r->data = tempEv;
            }
            return true;
        }
    }

    // EXPORT DATA FOR BROWSER / LOCALSTORAGE
    string export_all_data() const {
        vector<event> vec;
        collect_events(root, vec);
        string result = "";
        for (const auto& ev : vec) {
            result += ev.to_filestring() + "\n";
        }
        return result;
    }

    // LOAD DATA FROM BROWSER / LOCALSTORAGE
    void load_from_string(const string& data) {
        clear_tree();
        stringstream all(data);
        string line;
        while(getline(all, line))
        {
            if(line.empty()) continue;
            stringstream ss(line);
            string idstr, name, category, date, time, venue, organizer, registration_link;

            getline(ss, idstr, '|');
            getline(ss, name, '|');
            getline(ss, category, '|');
            getline(ss, date, '|');
            getline(ss, time, '|');
            getline(ss, venue, '|');
            getline(ss, organizer, '|');
            getline(ss, registration_link, '|');

            int num = stoi(idstr);
            event ev(num, name, category, date, time, venue, organizer, registration_link);
            insert(root, ev);
        }
    }

    // GENERATE HTML/CSS UI DASHBOARD
    void generate_web_ui(node* r) const
    {
        vector<event> vec;
        collect_events(r, vec);

        ofstream html("events_dashboard.html");
        html << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
        html << "<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        html << "<title>Centralized College Events Tracker</title>\n";
        html << "<link href=\"https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap\" rel=\"stylesheet\">\n";
        html << "<style>\n";
        html << "  :root { --bg: #f8fafc; --card-bg: #ffffff; --primary: #6366f1; --primary-hover: #4f46e5; --text: #1e293b; --text-light: #64748b; --border: #e2e8f0; }\n";
        html << "  body { font-family: 'Inter', sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 40px 20px; }\n";
        html << "  .container { max-width: 1200px; margin: 0 auto; }\n";
        html << "  header { text-align: center; margin-bottom: 40px; }\n";
        html << "  header h1 { font-size: 2.5rem; color: #0f172a; margin-bottom: 10px; font-weight: 700; }\n";
        html << "  header p { color: var(--text-light); font-size: 1.1rem; }\n";
        html << "  .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(340px, 1fr)); gap: 24px; }\n";
        html << "  .card { background: var(--card-bg); border-radius: 16px; padding: 24px; box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.05); border: 1px solid var(--border); display: flex; flex-direction: column; justify-content: space-between; }\n";
        html << "  .tag { display: inline-block; background: #e0e7ff; color: #4338ca; font-size: 0.75rem; font-weight: 600; padding: 4px 10px; border-radius: 20px; margin-bottom: 12px; text-transform: uppercase; }\n";
        html << "  .card h3 { margin: 0 0 12px 0; font-size: 1.25rem; color: #0f172a; }\n";
        html << "  .details { font-size: 0.9rem; color: var(--text-light); margin-bottom: 20px; line-height: 1.6; }\n";
        html << "  .details strong { color: var(--text); }\n";
        html << "  .btn { display: inline-block; text-align: center; background: var(--primary); color: white; padding: 10px 16px; border-radius: 8px; text-decoration: none; font-weight: 500; font-size: 0.9rem; }\n";
        html << "</style>\n</head>\n<body>\n";
        html << "<div class=\"container\">\n";
        html << "<header>\n<h1>🌟 Centralized College Events</h1>\n<p>Browse upcoming campus events, workshops, and register instantly.</p>\n</header>\n";
        html << "<div class=\"grid\">\n";

        if(vec.empty()) {
            html << "<p style=\"text-align:center; grid-column: 1/-1; color: #64748b;\">No upcoming events found.</p>";
        }

        for(const auto& ev : vec)
        {
            html << "<div class=\"card\">\n<div>\n";
            html << "<span class=\"tag\">" << ev.get_category() << "</span>\n";
            html << "<h3>" << ev.get_name() << "</h3>\n";
            html << "<div class=\"details\">\n";
            html << "📅 <strong>Date:</strong> " << ev.get_date() << " at " << ev.get_time() << "<br>\n";
            html << "📍 <strong>Venue:</strong> " << ev.get_venue() << "<br>\n";
            html << "👤 <strong>Organizer:</strong> " << ev.get_organizer() << "\n";
            html << "</div>\n</div>\n";
            html << "<a href=\"" << ev.get_registration_link() << "\" target=\"_blank\" class=\"btn\">Register / Link</a>\n";
            html << "</div>\n";
        }

        html << "</div>\n</div>\n</body>\n</html>";
        html.close();
        cout << "\n✨ HTML Dashboard generated successfully as 'events_dashboard.html'!" << endl;
    }
};

class WebBridge {
public:
    static string search_json(node* root, const string& query, const string& cat, const string& day, const string& sortBy) {
        vector<event> vec;
        bst tempBst;
        tempBst.collect_events(root, vec);

        string q = query;
        for (auto &c : q) c = (char)tolower((unsigned char)c);

        vector<event> filtered;
        for (const auto& e : vec) {
            string nameLower = e.get_name();
            for (auto &c : nameLower) c = (char)tolower((unsigned char)c);
            
            if (!q.empty() && nameLower.find(q) == string::npos && 
                e.get_category().find(cat) == string::npos && 
                e.get_date().find(day) == string::npos) 
                continue;
            
            if (cat != "" && cat != "All categories" && e.get_category() != cat) continue;
            if (day != "" && e.get_date() != day) continue;
            
            filtered.push_back(e);
        }

        if (sortBy == "name") {
            sort(filtered.begin(), filtered.end(), [](const event& a, const event& b) {
                return a.get_name() < b.get_name();
            });
        } else {
            sort(filtered.begin(), filtered.end(), [](const event& a, const event& b) {
                if (a.get_date() != b.get_date()) return a.get_date() < b.get_date();
                return a.get_time() < b.get_time();
            });
        }

        string json = "[";
        for (size_t i = 0; i < filtered.size(); ++i) {
            if (i > 0) json += ",";
            const auto& e = filtered[i];
            json += "{\"id\":" + to_string(e.get_id()) +
                    ",\"name\":\"" + e.get_name() +
                    "\",\"category\":\"" + e.get_category() +
                    "\",\"date\":\"" + e.get_date() +
                    "\",\"time\":\"" + e.get_time() +
                    "\",\"venue\":\"" + e.get_venue() +
                    "\",\"organizer\":\"" + e.get_organizer() +
                    "\",\"description\":\"\",\"link\":\"" + e.get_registration_link() + "\"}";
        }
        json += "]";
        return json;
    }

    static string get_event_json(node* root, int id) {
        node* curr = root;
        while (curr != nullptr) {
            if (id == curr->data.get_id()) {
                const auto& e = curr->data;
                return "{\"id\":" + to_string(e.get_id()) +
                       ",\"name\":\"" + e.get_name() +
                       "\",\"category\":\"" + e.get_category() +
                       "\",\"date\":\"" + e.get_date() +
                       "\",\"time\":\"" + e.get_time() +
                       "\",\"venue\":\"" + e.get_venue() +
                       "\",\"organizer\":\"" + e.get_organizer() +
                       "\",\"description\":\"\",\"link\":\"" + e.get_registration_link() + "\"}";
            }
            if (id < curr->data.get_id()) curr = curr->left;
            else curr = curr->right;
        }
        return "null";
    }
};

static bst globalTree;
static string webOutBuf;

extern "C"
{
    EMSCRIPTEN_KEEPALIVE void load_data(const char* data) {
        globalTree.load_from_string(data ? data : "");
    }

    EMSCRIPTEN_KEEPALIVE const char* export_data() {
        webOutBuf = globalTree.export_all_data();
        return webOutBuf.c_str();
    }

    EMSCRIPTEN_KEEPALIVE const char* search_events(const char* query, const char* category, const char* date, const char* sortBy) {
        webOutBuf = WebBridge::search_json(globalTree.get_root(), query ? query : "", category ? category : "", date ? date : "", sortBy ? sortBy : "date");
        return webOutBuf.c_str();
    }

    EMSCRIPTEN_KEEPALIVE const char* get_event(int id) {
        webOutBuf = WebBridge::get_event_json(globalTree.get_root(), id);
        return webOutBuf.c_str();
    }

    EMSCRIPTEN_KEEPALIVE int add_event(const char* name, const char* category, const char* date, const char* time, const char* venue, const char* organizer, const char* description, const char* link) {
        if (!name || string(name).empty()) return -1;
        int newId = 1;
        vector<event> vec;
        globalTree.collect_events(globalTree.get_root(), vec);
        for (const auto& e : vec) {
            if (e.get_id() >= newId) newId = e.get_id() + 1;
        }
        string sName = name, sCat = category, sDate = date, sTime = time, sVenue = venue, sOrg = organizer, sLink = link;
        event ev(newId, sName, sCat, sDate, sTime, sVenue, sOrg, sLink);
        globalTree.insert(globalTree.get_root(), ev);
        return newId;
    }

    EMSCRIPTEN_KEEPALIVE int update_event(int id, const char* name, const char* category, const char* date, const char* time, const char* venue, const char* organizer, const char* description, const char* link) {
        return 1;
    }

    EMSCRIPTEN_KEEPALIVE int delete_event(int id) {
        return globalTree.delete_event(globalTree.get_root(), id) ? 1 : 0;
    }
}

int main()
{
    node* root = globalTree.get_root();
    int mainChoice = 0;

    while (mainChoice != 3)
    {
        cout << "\n==============================================";
        cout << "\n  CENTRALIZED COLLEGE EVENTS TRACKER (C++)";
        cout << "\n==============================================";
        cout << "\n 1. Student Portal (Browse & Register)";
        cout << "\n 2. Organizer Portal (Manage Events)";
        cout << "\n 3. Exit";
        cout << "\n Select Portal (1-3): ";
        cin >> mainChoice;

        if (mainChoice == 1)            // WAY TO STUDENT PORTAL
        { 
            int studentChoice = 0;
            while (studentChoice != 4) 
            {
                cout << "\n\n--- STUDENT PORTAL ---";
                cout << "\n 1. View All Upcoming Events";
                cout << "\n 2. Search Event by ID";
                cout << "\n 3. Generate Web UI Dashboard";
                cout << "\n 4. Back to Main Menu";
                cout << "\n Choose an option (1-4): ";
                cin >> studentChoice;

                switch (studentChoice) 
                {
                    case 1:
                        globalTree.inorder(root);
                        break;
                    case 2: {
                        int id;
                        cout << "Enter Event ID: ";
                        cin >> id;
                        globalTree.search_event(root, id);
                        break;
                    }
                    case 3:
                        globalTree.generate_web_ui(root);
                        break;
                    case 4:
                        cout << "\nReturning to Main Menu...\n";
                        break;
                    default:
                        cout << "\nInvalid selection. Try again.\n";
                }
            }
        }
        else if (mainChoice == 2)           // WAY TO ORGANIZERS PORTAL
        { 
            int organizerChoice = 0;
            while (organizerChoice != 4) 
            {
                cout << "\n\n--- ORGANIZER PORTAL ---";
                cout << "\n 1. Add New Event";
                cout << "\n 2. Delete Event";
                cout << "\n 3. Edit Event";
                cout << "\n 4. Back to Main Menu";
                cout << "\n Choose an option (1-4): ";
                cin >> organizerChoice;

                if (organizerChoice == 1) {
                    globalTree.add_event(root);
                } 
                else if (organizerChoice == 2) {        
                    int deleteId;
                    cout << "\nEnter Event ID to Delete: ";
                    cin >> deleteId;
                    bool deleted = globalTree.delete_event(root, deleteId);
                    if(deleted){
                        cout<<"Event deleted successfully"<<endl;
                    }
                    else
                        cout<<"Event not found!"<<endl;
                }
                else if(organizerChoice == 3){       
                    cout << "\nEdit feature coming soon!\n";
                }
                else if (organizerChoice == 4) {
                    cout << "\nReturning to Main Menu...\n";
                } 
                else 
                    cout << "\nInvalid choice. Try again.\n";
            }
        }
        else if (mainChoice == 3) {
            cout << "\nThank you for using Centralized College Events Tracker!\n";
        } 
        else {
            cout << "\nInvalid choice. Please enter 1, 2, or 3.\n";
        }
    }
    return 0;
}