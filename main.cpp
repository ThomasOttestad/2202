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
// struct rating_data
// {
//     double rating;
//     double item;
// };
struct starting_queue_range
{
    int start;
    int end;
};







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



    // float stepsize;
    const int k = 100;
    const float alpha = 0.012;
    const float beta = 0.01;
    const float lambda = 0.05;
    float t = 5;


    matrix_size training_matrix_size = get_matrix_size(training_matrix_size);
    int n_users = training_matrix_size.users;
    int n_items = training_matrix_size.items;

    multimap<int, int> items;
    multimap<pair <int, int> , float> rating_map;


    cout << "amount of items in matrix" << training_matrix_size.items << endl;
    cout << "amount of users in matrix" << training_matrix_size.users << endl;

    std::default_random_engine generator;
    uniform_real_distribution<double> distribution(0.0,0.1);
    // uniform_real_distribution
    // uniform_int_distribution


    matrix_array *training_matrix = new matrix_array[training_matrix_size.users];// unordered map more space efficient
    for (int i = 0; i < training_matrix_size.users; i++)
    {
        training_matrix[i].index = i;
        // malloc(sizeof(double)*training_matrix_size.items)
        training_matrix[i].nested_array = new double[training_matrix_size.items];
    }


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
        float rating_from_data = stof(str_tmp);
        
        items.insert({item_idx,user_idx});
 
        rating_map.insert({{user_idx, item_idx},rating_from_data});

    }

    upcxx::init(); ////////////////////////////////////////////////////////////////////////////////

    upcxx::dist_object<upcxx::global_ptr<queue<int>>> item_queue(upcxx::new_ <queue<int>>());
    upcxx::dist_object<upcxx::global_ptr<double>> H_array(upcxx::new_array<double>((k * n_items)/upcxx::rank_n()));
    double *W_array = new double[k * n_users];
    for(int i = 0; i < n_users*k; i++)
    {
        W_array[i] = distribution(generator);
    }
    for(int i = 0; i < (k * n_items)/upcxx::rank_n(); i++)
    {
        H_array->local()[i] = (double)distribution(generator);
    }

    starting_queue_range starting_range = get_range(upcxx::rank_me(),n_users);
    while (starting_range.start < starting_range.end)
    {
        item_queue->local()->push(starting_range.start);
        starting_range.start++;
    }
    

    while(item_queue->local()->empty() != true)
    {
        int j = item_queue->local()->front();
        item_queue->local()->pop();
        
        // int user_index = current_array.index;

        auto item_index = items.find(j);

        float current_error = 0;
        for(int i = 0; i < k; i++)
        {
            cout << "W_array[i] " << W_array[i] << "H_array->local()[current_item] " << H_array->local()[j] << endl;
            current_error += (W_array[i] * H_array->local()[j]);
        }
        //calculate stepsize
        float S_t = alpha / (1 + beta * t);
        t*=1.1;




        cout << "current error " << current_error << endl;


        
    

    }

    //splitte mellom worker queue's
    // for(int i = 0)

    


    cout << "Hello world from process " << upcxx::rank_me()
    << " out of " << upcxx::rank_n() << " processes" << endl;
    upcxx::global_ptr<int> matrix_H = upcxx::new_<int>( upcxx::rank_me() );
    upcxx::dist_object<int> Worker(upcxx::rank_me());



    cout << "Hei" << endl;
    upcxx::finalize();

    //calc rmse

    return 0;
}