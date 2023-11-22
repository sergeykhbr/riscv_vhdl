/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <string.h>

static int foo_cnt = 0;

class Foo {
 public:
    Foo(const char *name) : idx_(0) {
        printf("Foo '%s', cnt = %d\n", name, ++foo_cnt);
    }

 private:
    int idx_;
};


/**
   WARNING: This example requires at least 128 KB ROM (default is 64 KB. Check RTL).
*/
extern "C" int main() {
    Foo A("A");
    Foo B("B");

    printf("End of test %s\n", __DATE__);

    while (1) {}
    return 0;
}
