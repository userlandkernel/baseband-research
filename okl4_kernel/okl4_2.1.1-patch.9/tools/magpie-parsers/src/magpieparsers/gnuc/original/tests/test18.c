
     concat_fopen (char *s1, char *s2, char *mode)
     {
       char str[strlen (s1) + strlen (s2) + 1];
       strcpy (str, s1);
       strcat (str, s2);
       return fopen (str, mode);
     }
