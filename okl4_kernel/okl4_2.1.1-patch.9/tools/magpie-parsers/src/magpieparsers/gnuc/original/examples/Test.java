import java.io.*;

import antlr.*;

public class Test
{
    public static void main(String[] args)
    {
        for (int i=0; i<args.length; i++)
        {
        try
            {
            String programName = args[i];
//      System.out.println("\nworking on " + programName);
//      System.out.flush();
//      System.err.println("\nworking on " + programName);
//      System.err.flush();
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
            // Parse the input expression.
            GnuCParser parser = new GnuCParser ( lexer );
            
            // set AST node type to TNode or get nasty cast class errors
            parser.setASTNodeType(TNode.class.getName());
            TNode.setTokenVocabulary("GNUCTokenTypes");

            // invoke parser
            try {
                parser.translationUnit();
            }
            catch (RecognitionException e) {
                System.err.println("Fatal IO error:\n"+e);
                System.exit(1);
            }
            catch (TokenStreamException e) {
                System.err.println("Fatal IO error:\n"+e);
                System.exit(1);
            }

            // Garbage collection hint
            System.gc();
            
//      System.out.println(lexer.getPreprocessorInfoChannel());
//    TNode.printTree(parser.getAST());      System.out.flush();  

            // Garbage collection hint
            System.gc();

            GnuCEmitter e = new GnuCEmitter(lexer.getPreprocessorInfoChannel());
            
            // set AST node type to TNode or get nasty cast class errors
            e.setASTNodeType(TNode.class.getName());

            // walk that tree
            e.translationUnit( parser.getAST() );

            // Garbage collection hint
            System.gc();

            }
        catch ( Exception e )
            {
            System.err.println ( "exception: " + e);
            e.printStackTrace();
            }
        }
    }
}

