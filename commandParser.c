#include "utilities.h"
#include "builtin.h"
#include "programs.h"

// displays welcome message when shell is started
void welcomeMessage() {
    printf("\t                             __          __ __ \n");
    printf("\t .-----.-----.--.--.--.-----|  |--.-----|  |  |\n");
    printf("\t |     |  -__|  |  |  |__ --|     |  -__|  |  |\n");
    printf("\t |__|__|_____|________|_____|__|__|_____|__|__|\n");
    printf("\n");         
}

// displays the command line prompt
void cmdPrompt() {
    printf("~$ ");
}

// parse command line buffer, call the specified command functions
void cmdLineReader(char cmdBuffer[]) {
    // Remove newline from the end of cmdBuffer
    int length = strlen(cmdBuffer);
    if (length > 0 && cmdBuffer[length - 1] == '\n')
        cmdBuffer[--length] = '\0';

    // Tokenize cmdBuffer string into individual strings, using semicolon as delimiter
    char* parsedCmdBuffer[BUFFER_MAX];
    memset(parsedCmdBuffer, 0, sizeof(parsedCmdBuffer));
    char* token = strtok(cmdBuffer, ";");
    int count = 0;

    // Store tokens into parsedCmdBuffer array
    while (token != NULL) {
        parsedCmdBuffer[count] = token;
        token = strtok(NULL, ";");  // Get next token
        count++;
    }

    // Variables used to execute exit command last
    int lastCommand = 0;
    int exitCalled = 0;

    // Loop through each parsed command
    for (int i = 0; i < count; i++) {
        // Check if input contains redirection operators for the current command
        char* inputFile = strstr(parsedCmdBuffer[i], " < ");
        char* outputFile = strstr(parsedCmdBuffer[i], " > ");

        // Store original file descriptors for stdin and stdout
        int stdin_copy = dup(STDIN_FILENO);
        int stdout_copy = dup(STDOUT_FILENO);

        if (inputFile != NULL) {
            // Input redirection detected
            *inputFile = '\0'; // Null terminate before '<'
            char *fileName = inputFile + 3; // Points to the filename after '<'
            handleInputRedirection(fileName);
        }

        if (outputFile != NULL) {
            // Output redirection detected
            *outputFile = '\0'; // Null terminate before '>'
            char *fileName = outputFile + 3; // Points to the filename after '>'
            handleOutputRedirection(fileName);
        }

        // Tokenize the current command string into individual arguments
        char* currentCmd[BUFFER_MAX];
        memset(currentCmd, 0, sizeof(currentCmd));
        token = strtok(parsedCmdBuffer[i], " ");
        int count2 = 0;

        // Store tokens into currentCmd array
        while (token != NULL) {
            currentCmd[count2] = token;
            token = strtok(NULL, " "); // Get next token
            count2++;
        }

        // Execute the current command
        if (currentCmd[0] != NULL) {
            executeCmds(currentCmd);
        }

        // Restore original stdin and stdout
        dup2(stdin_copy, STDIN_FILENO);
        dup2(stdout_copy, STDOUT_FILENO);

        // Check if current command iteration is last
        if ((i + 1) == count)
            lastCommand = 1;

        // Call exit if previously skipped
        if (lastCommand && exitCalled)
            handleExitCmd();
    }
}
