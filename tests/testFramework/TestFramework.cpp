//
// Created by Ethan Horowitz on 11/5/23.
//
#include "TestFramework.h"
#include <vector>
#include <map>
#include <string>

typedef struct Test{
    void (*test)();
    const char* name;
} Test;

std::map<std::string, std::vector<Test>*>* testGroups;
bool errorFlag = false;

void __addTest(void (*test)(), const char* group, const char* name){
    if (!testGroups){
        testGroups = new std::map<std::string, std::vector<Test>*>();
    }
    if(testGroups->find(group) == testGroups->end()){
        auto* tests = new std::vector<Test>();
        std::string s(group);
        (*testGroups)[s] = tests;
    }
    (*testGroups)[group]->push_back({test, name});

}


void __setErrorFlag(){
    errorFlag = true;
}

void RUN_TESTS(){
    int num_errors = 0;
    int total_tests = 0;
    for(auto& group : *testGroups){
        printf("Running tests in group %s\n", group.first.c_str());
        for(Test& test : *group.second){
            errorFlag = false;
            test.test();
            if (errorFlag) {
                printf("\tx  ");
                num_errors++;
            }
            else {
                printf("\t✓  ");
            }
            total_tests++;
            printf("%s\n", test.name);

        }
    }
    printf("Test results: %d successes, %d fails\n", total_tests - num_errors, num_errors);
}

void RUN_TEST_GROUP(std::string group){
    int num_errors = 0;
    int total_tests = 0;
    for(auto& test : *((*testGroups)[group])){
        errorFlag = false;
        test.test();
        if (errorFlag) {
            printf("\tx  ");
            num_errors++;
        }
        else
            printf("\t✓  ");
        printf("%s\n", test.name);
        total_tests++;
    }
    printf("Test results for group %s: %d successes, %d fails\n", group.c_str(), total_tests - num_errors, num_errors);
}

__TFTest::__TFTest(void (*test)(), const char* group, const char* name){
    __addTest(test, group, name);
}