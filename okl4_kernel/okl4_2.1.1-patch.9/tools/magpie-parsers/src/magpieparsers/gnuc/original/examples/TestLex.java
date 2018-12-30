import java.io.*;

import antlr.CommonAST;
import antlr.DumpASTVisitor;
import antlr.Token;
public class TestLex
{
    public static void main(String[] args)
    {
        for (int i=0; i<args.length; i++)
        {
        try
            {
            String programName = args[i];
            DataInputStream dis = null;
            if (programName.equals("-")) {
                dis = new DataInputStream( System.in );
            }   
            else {
                dis = new DataInputStream(new FileInputStream(programName));
            }
            GnuCLexer lexer =
                new GnuCLexer ( dis );
            lexer.setTokenObjectClass("CToken");
            lexer.initialize();
            Token t= lexer.nextToken();
            while (t.getType() != 1 ) {
                System.out.println( t.getText() );
                t = lexer.nextToken();
            }

            }
        catch ( Exception e )
            {
            System.err.println ( "exception: " + e);
            e.printStackTrace();
            }
        }
    }
}
        

