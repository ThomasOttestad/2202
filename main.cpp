#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <queue>
#include <iostream>
#include <random>
#include <upcxx/upcxx.hpp>
#include "eigen/Eigen/Dense"

#include <sys/time.h>
#include <sys/resource.h>

using namespace std;
using namespace Eigen;


struct queue_node
{
    int index;
    struct queue_node* next;
};
struct worker_queue
{
    queue_node* start;
    queue_node* end;
};


struct worker{
    std::queue<int> worker_queue;
    
};
struct matrix_size{
    int items;
    int users;
};
struct matrix_array
{
    int index;
    double *nested_array;
};
struct rating_data
{
    double rating;
    double item;
};
struct starting_queue_range
{
    int start;
    int end;
};




// matrix_array *importData(const char *filename,int n_items,int n_users,matrix_array *new_array){
//     string delimiter = " ";

//     ifstream data;
//     data.open(filename);

//     string row;

//     // MatrixXd training_set = MatrixXd::Zero(n_users, n_items);
    
//     // Resets getline function to start of file
//     data.clear();
//     data.seekg(0, ios::beg);
//     int x = 0;

//     // Adds all rating to the matrix using user and items in a row as index
//     while(getline(data, row)){
//         string str_tmp = row.substr(0, row.find(delimiter));
//         int user_idx = stoi(str_tmp);
//         cout << "user_idx " << user_idx << endl;

//         int len = str_tmp.length() + 1;
//         cout << "len " << len << endl;
//         str_tmp = row.substr(len, row.find(delimiter, len));
//         int item_idx = stoi(str_tmp);
//         cout << "item_ idx " << item_idx << endl;

        
//         str_tmp = row.substr(row.find(delimiter, len), row.find("\n"));
//         float rating = stof(str_tmp);
//         cout << "str_tmp " << str_tmp << " and rating " << rating << endl;
        

//         // training_set(user_idx - 1, item_idx - 1) = rating;
//   //
//   //    må legge til et system for å få all dataen på --rating
//   //

//         // new_array[user_idx - 1].array[item_idx-1] = rating;
//     }

//     return (matrix_array*)new_array;
// }




int get_data_from_matrix()
{

}



int creat_workers()
{
    
}

int calculate_RMSE()
{
     


}
worker_queue* create_queue() 
{ 
    worker_queue* q = (worker_queue*)malloc(sizeof(worker_queue)); 
    q->start = q->end = NULL; 
    return q; 
} 
void addto_queue(worker_queue *w_q, int value)
{
    queue_node* new_node = (queue_node *)malloc(sizeof(queue_node));
    new_node->index = value;
    if (w_q->end == NULL)
    {
        new_node->next = NULL;
        w_q->start = new_node;
        w_q->end = new_node;
        return;
    }
    new_node->next = w_q->end;
    w_q->end = new_node;
}
queue_node* remove_from_queue(worker_queue* w_q)
{
    if(w_q->start == NULL) return NULL;
    queue_node *temp = w_q->start;
    w_q->start = w_q->start->next;
    if(w_q->start == NULL)
    {
        w_q->end == NULL;
    }
    return temp;
}

matrix_size get_matrix_size(matrix_size matrix)
{
    string delimiter = " ";

    ifstream data;
    data.open("Data_for_SGD/train_small.txt");

    string row;
    float n_users = 0;
    float n_items = 0;

    // Calcualtes dimension of matrix by finding highest values of user and item
    while(getline(data, row)){
        string str_tmp = row.substr(0, row.find(delimiter));
        float n = stof(str_tmp);
        if(n > n_users){
            n_users = n;
        }

        str_tmp = row.substr(str_tmp.length() + 1, row.find(delimiter));
        n = stof(str_tmp);
        if(n > n_users){
            n_items = n;
        }
    }
    matrix.items = n_items;
    matrix.users = n_users;
    return matrix;
}

starting_queue_range get_range(int rank, int max)
{
    starting_queue_range result;
    result.start = (int)rank * (max / upcxx::rank_n()-1);
    result.end = (int)(rank + 1) * (max / upcxx::rank_n()-1);
    return result;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]){

    cout << "start" << endl;
    cout << argc << endl;
    cout << argv[0] << endl;

    
    matrix_size training_matrix_size = get_matrix_size(training_matrix_size);

    cout << "amount of items in matrix" << training_matrix_size.items << endl;
    cout << "amount of users in matrix" << training_matrix_size.users << endl;

    std::default_random_engine generator;
    uniform_real_distribution<double> distribution(0.0,1.0);
    // uniform_real_distribution
    // uniform_int_distribution


    matrix_array *training_matrix = new matrix_array[training_matrix_size.users];// unordered map more space efficient
    for (int i = 0; i < training_matrix_size.users; i++)
    {
        training_matrix[i].index = i;
        // malloc(sizeof(double)*training_matrix_size.items)
        training_matrix[i].nested_array = new double[training_matrix_size.items];
    }
    // *training_matrix = importData("Data_for_SGD/train_small.txt",training_matrix_size.items,training_matrix_size.users,training_matrix);
    // // user 1 item 1
    // cout << training_set(0, 0) << endl;
    // // user 1 item 1029
    // cout << training_set(0, 1028) << endl;
    string delimiter = " ";

    ifstream data;
    data.open("Data_for_SGD/train_small.txt");

    string row;

    // MatrixXd training_set = MatrixXd::Zero(n_users, n_items);
    
    // Resets getline function to start of file
    data.clear();
    data.seekg(0, ios::beg);
    int x = 0;


    // Adds all rating to the matrix using user and items in a row as index
    while(getline(data, row)){
        // cout << "start of loop" << endl;
        string str_tmp = row.substr(0, row.find(delimiter));
        int user_idx = stoi(str_tmp);
        // cout << "user_idx " << user_idx << endl;

        int len = str_tmp.length() + 1;
        // cout << "len " << len << endl;
        str_tmp = row.substr(len, row.find(delimiter, len));
        int item_idx = stoi(str_tmp);

        
        str_tmp = row.substr(row.find(delimiter, len), row.find("\n"));
        float rating = stof(str_tmp);
            // cout << "str_tmp " << str_tmp << " and rating " << rating << endl;
            // cout << "item_ idx " << item_idx << " user_idx " << user_idx << endl;
            // cout << " row " << row << endl;
        // cout << "array size  " << sizeof(training_matrix[user_idx -1].array) << endl;
        // int size = sizeof(training_matrix[user_idx -1].array[])
        //  cout << " previos rating " << training_matrix[user_idx - 1].nested_array[item_idx-1]
        training_matrix[user_idx - 1].nested_array[item_idx-1] = rating;
    }

    cout << "making W and H matrix !!!!!!!" << endl;


    matrix_array *W_matrix = new matrix_array[training_matrix_size.items];
    // // user 11 item 88
    for(int i = 0; i < training_matrix_size.items; i++)
    {
        W_matrix[i].index = i;
        // malloc(sizeof(double)*training_matrix_size.items)
        W_matrix[i].nested_array = new double[2];
        // cout << " i " << i << endl;
        for(int j = 0; j < 2; j++)
        {
            // cout << " j " << j << endl;
            // float temp = (float)1/sqrt((int)training_matrix_size.items);
            // cout << "value at index " << W_matrix[i].nested_array[j] << endl;
            // cout << " temp " << temp << endl;
            // W_matrix[i].nested_array[j] = (double)((float)rand() % temp);
            W_matrix[i].nested_array[j] = distribution(generator);
        }
    }
    cout << " laget W_matrix" << endl;

    matrix_array *H_matrix = new matrix_array[2];

    for(int i = 0; i < 2; i++)
    {
        H_matrix[i].index = i;
        // malloc(sizeof(double)*training_matrix_size.items)
        H_matrix[i].nested_array = new double[training_matrix_size.users];
        for(int j = 0; j < training_matrix_size.users; j++)
        {
            // int temp = (int)1/sqrt((int)training_matrix_size.items);
            // H_matrix[i].nested_array[j] = (double)(rand() % temp);
            H_matrix[i].nested_array[j] = distribution(generator);
        }
    }

    cout << " laget H_matrix" << endl;

    //create W and H matrix

    cout << "starting upcxx !!!!!!!" << endl;
    upcxx::init();
    upcxx::dist_object<upcxx::global_ptr<queue<matrix_array>>> worker_queues(upcxx::new_ <queue<matrix_array>>());
    starting_queue_range starting_range = get_range(upcxx::rank_me(),training_matrix_size.users);
    while (starting_range.start < starting_range.end)
    {
        worker_queues->local()->push(training_matrix[starting_range.start]);
        starting_range.start++;
    }
    

    while(worker_queues->local()->empty() != true)
    {
        matrix_array* current_array = worker_queues->local()->pop();
        
    }

    //splitte mellom worker queue's
    // for(int i = 0)

    


    cout << "Hello world from process " << upcxx::rank_me()
    << " out of " << upcxx::rank_n() << " processes" << endl;
    upcxx::global_ptr<int> matrix_H = upcxx::new_<int>( upcxx::rank_me() );
    upcxx::dist_object<int> Worker(upcxx::rank_me());



    cout << "Hei" << endl;
    upcxx::finalize();
    return 0;
}