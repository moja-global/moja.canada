/**
 * @file 
 * @brief This is the main file used here.
 * 
 * Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum
 * ******/

#include "../include/moja/modules/cbm/main.h"
#include<iostream>

namespace a{
    namespace b{
        namespace c{
            /**
             * @brief Constructor.
             * ****************/

            pls::pls(){
                std::cout<< "This is the constructor " << "\n";
            }

            /**
             * @brief Sum.
             * 
             * This calculated the sum of 2 numbers
             * 
             * @param a int
             * @param b int
             * @return int
             * ****************/
            int pls::add(int a, int b){
                return (a + b);
            }
        }
    }
}
