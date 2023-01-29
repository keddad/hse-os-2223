#!/bin/bash

greet_file () {
    echo "Hello, $1" >> hello
}

greet_file $1