typedef int T1 ;
typedef int T2 ;
typedef int T3;
main() {
        const T1 T1; /* redeclares T1 to be an int */
        const T2 (T2); /* redeclares T2 to be an int */
        const T3; /* syntax error : missing declarator */
        }

