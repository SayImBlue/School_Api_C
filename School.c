#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <bcrypt.h> 

#define STRING_MAX 100
int Is_Student = -1;
int SuperId;
int Is_Logged = 0;


int GetID(sqlite3* db, char* Mail)
{
    int MiniId = -1;
    int rc;
    const char* SqlStd = "SELECT StdID FROM Credentials WHERE Mail = ?;";
    const char* SqlEmp = "SELECT ID FROM Credentials WHERE Mail = ?;";
    sqlite3_stmt* stmt;

    //Preparing the statements depending on the type of user
    if(Is_Student == 1)
    {
        rc = sqlite3_prepare_v2(db, SqlStd, -1, &stmt, NULL);
        if(rc != SQLITE_OK)
        {
            fprintf(stderr, "Error while preparing the statement : %s\n", sqlite3_errmsg(db));
            return -1;
        }
    }
    else if(Is_Student == 0)
    {
        rc = sqlite3_prepare_v2(db, SqlEmp, -1, &stmt, NULL);
        if(rc != SQLITE_OK)
        {
            fprintf(stderr, "Error while preparing the statement : %s\n", sqlite3_errmsg(db));
            return -1;
        }
    }
    else
    {
        printf("Error occured");
        return -1;
    }
    //Binding the mail to the statement
    sqlite3_bind_text(stmt, 1, Mail, -1, SQLITE_TRANSIENT);

    //Executing the query
    rc = sqlite3_step(stmt);
    if(rc == SQLITE_ROW) MiniId = sqlite3_column_int(stmt, 0);
    else printf("No data found");
    sqlite3_finalize(stmt);
    return MiniId;
}

int Register(sqlite3* db)
{
    char* Mail = (char*)malloc(STRING_MAX * sizeof(char));
    char* Add = (char*)malloc(STRING_MAX * sizeof(char));
    char* Dom = (char*)malloc(STRING_MAX * sizeof(char));
    char* Key = (char*)malloc(STRING_MAX * sizeof(char));
    char* Key2 = (char*)malloc(STRING_MAX * sizeof(char));
    char* Str = (char*)malloc(STRING_MAX * sizeof(char));
    char* Name = (char*)malloc(STRING_MAX * sizeof(char));
    char salt[BCRYPT_HASHSIZE];
    const char* sqlEmp = "INSERT INTO Credentials (ID, Mail, Pass) VALUES ((SELECT IFNULL(MAX(ID), 0) + 1 FROM Credentials), ?1, ?2);";
    const char* sqlStd = "INSERT INTO Credentials (StdID, Mail, Pass) VALUES ((SELECT IFNULL(MAX(StdID), 0) + 1 FROM Credentials), ?1, ?2);";
    const char* sqlEmp2 = "INSERT INTO Employees (ID, EmpName, EmpAge, EmpPosition, EmpNum, Admin) VALUES (?1, ?2, ?3, 'Undefined', ?4, 'No');";
    const char* sqlStd2 = "INSERT INTO Students (StdID, StdName, StdAge, StdMajor, StdNum, AVGGrade) VALUES (?1, ?2, ?3, 'Undefined', ?4, NULL);";
    sqlite3_stmt* stmt;
    sqlite3_stmt* stmt2;
    int rc, rc2, Age, Num;
    int i = 0;
    
    printf("Welcome to EuSchool's website, please enter your mail address in order to register : ");
    getchar();
    fgets(Str, STRING_MAX, stdin);
    Str[strcspn(Str, "\n")] = '\0';
    sscanf(Str,"%50[^@]@%50[^.].%50s",Add, Mail, Dom);
    memset(Str, '\0',STRING_MAX);
    //Valid e-mail verification
    while (!((strcmp(Mail, "gmail") == 0) || (strcmp(Mail, "outlook") == 0) || (strcmp(Mail, "yahoo") == 0)) || 
        !((strcmp(Dom, "com") == 0) || (strcmp(Dom, "org") == 0) || (strcmp(Dom, "ma") == 0) || (strcmp(Dom, "fr") == 0)))
    {
        printf("Invalid mail provider or domain, please try again: ");
        fgets(Str, STRING_MAX, stdin);
        sscanf(Str, "%50[^@]@%50[^.].%50s", Add, Mail, Dom);  
        memset(Str, '\0', STRING_MAX);
        i++;
        if(i == 5)
        {
            printf("Too many attempts, you will be logged off\n");
            return 0;
        }
    }
    //Concatenating the parts of the e-mail address into one string
    strcat(Add, "@");
    strcat(Add, Mail);
    strcat(Add, ".");
    strcat(Add, Dom);
    free(Mail);
    free(Dom);
    printf("Valid E-mail address ! Please enter a valid password : ");
    fgets(Str, STRING_MAX, stdin);
    strncpy(Key, Str, STRING_MAX);
    Key[strcspn(Key, "\n")] = '\0';
    memset(Str, '\0', STRING_MAX);
    i = 0;
    //Short passwords verfication
    while(strlen(Key) < 8)
    {
        printf("Password too short, must at least contain 8 characters\n");
        printf("Enter a new password that is at least 8 characters long : ");
        fgets(Str, STRING_MAX, stdin);
        strncpy(Key, Str, STRING_MAX);
        Key[strcspn(Key, "\n")] = '\0';
        memset(Str, '\0', STRING_MAX);
        i++;
        if(i == 5)
        {
            printf("Too many attempts, you will be logged off\n");
            return 0;
        }
            
    }
    printf("Re-enter your password to verify it : ");
    fgets(Str, STRING_MAX, stdin);
    strncpy(Key2, Str, STRING_MAX);
    Key2[strcspn(Key2, "\n")] = '\0';
    memset(Str, '\0', STRING_MAX);
    i = 0;
    //First && Second Password matching verification
    while(strcmp(Key, Key2) != 0)
    {
        printf("The first password doesnt match the second one, please re-enter the second password : ");
        fgets(Str, STRING_MAX, stdin);
        strncpy(Key2, Str, STRING_MAX);
        Key2[strcspn(Key2, "\n")] = '\0';
        memset(Str, '\0', STRING_MAX);
        i++;
        if(i == 5)
        {
            printf("Too many attempts, you will be logged off\n");
            return 0;
        }
    }
    memset(Key2, '\0', STRING_MAX);

    //Preparing the statements depending on the type of user
    if(Is_Student == 1)
    {
        rc = sqlite3_prepare_v2(db, sqlStd, -1, &stmt, NULL);
        rc2 = sqlite3_prepare_v2(db, sqlStd2, -1, &stmt2, NULL);
        if(rc != SQLITE_OK || rc2 != SQLITE_OK)
        {
            fprintf(stderr, "Error while preparing the statements : %s\n", sqlite3_errmsg(db));
            return -1;
        }
    }
    else if(Is_Student == 0)
    {
        rc = sqlite3_prepare_v2(db, sqlEmp, -1, &stmt, NULL);
        rc2 = sqlite3_prepare_v2(db, sqlEmp2, -1, &stmt2, NULL);
        if(rc != SQLITE_OK || rc2 != SQLITE_OK)
        {
            fprintf(stderr, "Error while preparing the statementS : %s\n", sqlite3_errmsg(db));
            return -1;
        }
    }
    else
    {
        printf("Invalid state");
        return -1;
    }

    //Generating the salt
    rc = bcrypt_gensalt(12, salt);
    if(rc != 0)
    {
        fprintf(stderr, "Error while generating the salt : %d\n", rc);
        return -1;
    }
    //Hashing the password
    rc = bcrypt_hashpw(Key, salt, Key2);
    if(rc != 0)
    {
        fprintf(stderr, "Error while hashing the password : %d\n", rc);
        return -1;
    }
    //Inputting the name and other infos
    i = 0;
    printf("To finalize your registration, please provide the following informations : \n");
    printf("Provide your full name : ");
    fgets(Str, STRING_MAX, stdin);
    strncpy(Name, Str,STRING_MAX);
    Name[strcspn(Name, "\n")] = '\0';
    memset(Str, '\0', STRING_MAX);
    printf("Great ! Now your age : ");
    fgets(Str, STRING_MAX, stdin);
    sscanf(Str, "%3d", &Age);
    memset(Str, '\0', STRING_MAX);
    //Valid Age Check
    while(Age > 120 || Age < 0)
    {
        printf("Invalid Age, please try again : ");
        fgets(Str, STRING_MAX, stdin);
        sscanf(Str, "%3d", &Age);
        memset(Str, '\0', STRING_MAX);
        i++;
        if(i == 5)
        {
            printf("Too many attempts, you will be logged off\n");
            return 0;
        }
    }
    printf("Amazing ! Finally, please provide your Moroccan phone number : ");
    fgets(Str, STRING_MAX, stdin);
    Str[strcspn(Str, "\n")] = '\0';
    while(strlen(Str) != 10)
    {
        memset(Str, '\0', STRING_MAX);
        printf("Invalid Number, it must be 10 digits long, please try again : ");
        fgets(Str, STRING_MAX, stdin);
        Str[strcspn(Str, "\n")] = '\0';
        i++;
        if(i == 5)
        {
            printf("Too many attempts, you will be logged off\n");
            return 0;
        }
    }
    sscanf(Str, "%10d", &Num);
    printf("Valid phone number !\n");
    printf("Thank you for your cooperation !\n");
    
    //Binding the mail and password 
    sqlite3_bind_text(stmt, 1, Add, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, Key2, -1, SQLITE_TRANSIENT);
    

    //Executing the Query 1
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE)
    {
        fprintf(stderr, "Error while executing the queries: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }
    printf("Added Entry successfully\n");
    sqlite3_finalize(stmt);

    //Binding the otther infos
    SuperId = GetID(db, Add);
    sqlite3_bind_int(stmt2, 1, SuperId);
    sqlite3_bind_text(stmt2, 2, Name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt2, 3, Age);
    sqlite3_bind_int(stmt2, 4, Num);

    //Executing the Query 2
    rc2 = sqlite3_step(stmt2);
    if(rc2 != SQLITE_DONE)
    {
        fprintf(stderr, "Error while executing the queries: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt2);
        return -1;
    }
    printf("Registration complete and successful\n");
    sqlite3_finalize(stmt2);

    free(Add);
    free(Key2);
    free(Key);
    free(Str);
    free(Name);
    return 1;
}

int Modify(sqlite3* db)
{
    char* Str = (char*)malloc(STRING_MAX * sizeof(char));
    char* Name = (char*)malloc(STRING_MAX * sizeof(char));
    char* Position = (char*)malloc(STRING_MAX * sizeof(char));
    char* Major = (char*)malloc(STRING_MAX * sizeof(char));
    char* Mail = (char*)malloc(STRING_MAX * sizeof(char));
    char* Add = (char*)malloc(STRING_MAX * sizeof(char));
    char* Dom = (char*)malloc(STRING_MAX * sizeof(char));
    char* Key = (char*)malloc(STRING_MAX * sizeof(char));
    char* Key2 = (char*)malloc(STRING_MAX * sizeof(char));
    char salt[BCRYPT_HASHSIZE];
    int Age, Num, Salary, rc, Choice, Choice2, ChoiceID, i = 0;
    float AvgGrade;
    int Admin = 0;
    const char* sqlCheck = "SELECT Admin FROM Employees WHERE ID = ?;";
    const char* sqlEmpName0 = "UPDATE Employees SET EmpName = ?1 WHERE ID = ?2;";
    const char* sqlEmpAge0 = "UPDATE Employees SET EmpAge = ?1 WHERE ID = ?2;";
    const char* sqlEmpNum0 = "UPDATE Employees SET EmpNum = ?1 WHERE ID = ?2;";
    const char* sqlStdName0 = "UPDATE Students SET StdName = ?1 WHERE StdID = ?2;";
    const char* sqlStdAge0 = "UPDATE Students SET StdAge = ?1 WHERE StdID = ?2;";
    const char* sqlStdNum0 = "UPDATE Students SET StdNum = ?1 WHERE StdID = ?2;";
    const char* sqlEmpSalary1 = "UPDATE Employees SET Salary = ?1 WHERE ID = ?2;";
    const char* sqlEmpPosition1 = "UPDATE Employees SET EmpPosition = ?1 WHERE ID = ?2;";
    const char* sqlStdMajor1 = "UPDATE Students SET StdMajor = ?1 WHERE StdID = ?2;";
    const char* sqlStdAvgGrade1 = "UPDATE Students SET AvgGrade = ?1 WHERE StdID = ?2;";
    const char* sqlEmpMail2 = "UPDATE Credentials SET Mail = ?1 WHERE ID = ?2;";
    const char* sqlEmpPass2 = "UPDATE Credentials SET Pass = ?1 WHERE ID = ?2;";
    const char* sqlStdMail2 = "UPDATE Credentials SET Mail = ?1 WHERE StdID = ?2;";
    const char* sqlStdPass2 = "UPDATE Credentials SET Pass = ?1 WHERE StdID = ?2;";
    sqlite3_stmt* stmtCheck;
    sqlite3_stmt* stmt;

    //Checking if the employee is an Admin or not
    if(Is_Student == 0)
    {
        //Preparing the query
        rc = sqlite3_prepare_v2(db, sqlCheck, -1, &stmtCheck, NULL);
        if(rc != SQLITE_OK)
        {
            fprintf(stderr, "Error while preparing the statement : %d\n", rc);
            return -1;
        }
        //Binding the statement
        sqlite3_bind_int(stmtCheck, 1, SuperId);

        //Executing the statement
        rc = sqlite3_step(stmtCheck);
        if(rc == SQLITE_ROW) 
        {
        //Retrives the Admin status
        strncpy(Str, (const char*) sqlite3_column_text(stmtCheck, 0), STRING_MAX);
        Str[strcspn(Str, "\n")] = '\0';
        sqlite3_finalize(stmtCheck);
        }
        else if(rc == SQLITE_DONE) 
        {
        printf("No valid entry found");
        sqlite3_finalize(stmtCheck);
        return 0;
        }
        else
        {
        fprintf(stderr, "Query execution failed : %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmtCheck);
        return -1;
        }
        //Checks if super admin or just normal admin
        if(strcmp(Str, "Yes") == 0)
        {
            printf("Welcome back dear admin !\n");
            Admin = 1;
        }
        else if(strcmp(Str, "Super") == 0)
        {
            printf("Super admin ! How have you been ?\n");
            Admin = 2;
        }
        memset(Str, '\0', STRING_MAX);
    }

    if(Is_Student == 0) //For employees only
    {    //Printing the different options
        printf("Choose an option : ");
        printf("\n 1 - Edit Name \t 2 - Edit Age \t 3 - Edit Number\t4 - Exit");
        if(Admin == 1)
        {
            printf("\n5 - Edit an employee's infos \t 6 - Edit a student's infos");
        }
        if(Admin == 2)
        {
            printf("\n5 - Edit an employee's infos \t 6 - Edit a student's infos \t 7 - Edit credentials infos \t");
        }
        printf("\n Option : ");
        fgets(Str, 2, stdin);
        sscanf(Str, "%1d", &Choice);
        memset(Str, '\0', STRING_MAX);
        
        //Actual choice switch case
        switch(Choice)
        {
            case 1 : //Modify name
            
            //Prepare statement
            rc = sqlite3_prepare_v2(db, sqlEmpName0, -1, &stmt, NULL);
            if(rc != SQLITE_OK)
            {
                fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                return -1;
            }
            getchar();
            printf("Provide your full name : ");
            fgets(Str, STRING_MAX, stdin);
            strncpy(Name, Str,STRING_MAX);
            Name[strcspn(Name, "\n")] = '\0';
            memset(Str, '\0', STRING_MAX);

            //Binding statement
            sqlite3_bind_text(stmt, 1, Name, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, SuperId);

            //Execute the query
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                sqlite3_reset(stmt);
                sqlite3_finalize(stmt);
                return -1;
            }
            printf("Modification successful\n");
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            break;

            case 2 : //Modify Age
            
            //Prepare statement
            rc = sqlite3_prepare_v2(db, sqlEmpAge0, -1, &stmt, NULL);
            if(rc != SQLITE_OK)
            {
                fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                return -1;
            }
            getchar();
            printf("Please enter your age : ");
            fgets(Str, STRING_MAX, stdin);
            sscanf(Str, "%3d", &Age);
            memset(Str, '\0', STRING_MAX);
            //Valid Age Check
            i = 0;
            while(Age > 120 || Age < 0)
            {
                printf("Invalid Age, please try again : ");
                fgets(Str, STRING_MAX, stdin);
                sscanf(Str, "%3d", &Age);
                memset(Str, '\0', STRING_MAX);
                i++;
                if(i == 5)
                {
                    printf("Too many attempts, you will be logged off\n");
                    return 0;
                }
            }   

            //Binding statement
            sqlite3_bind_int(stmt, 1, Age);
            sqlite3_bind_int(stmt, 2, SuperId);

            //Execute the query
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                sqlite3_reset(stmt);
                sqlite3_finalize(stmt);
                return -1;
            }
            printf("Modification successful\n");
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            break;

            case 3 : //Modify number

            //Prepare statement
            rc = sqlite3_prepare_v2(db, sqlEmpNum0, -1, &stmt, NULL);
            if(rc != SQLITE_OK)
            {
                fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                return -1;
            }

            //Phone number validation
            getchar();
            printf("Please provide your Moroccan phone number : ");
            fgets(Str, STRING_MAX, stdin);
            Str[strcspn(Str, "\n")] = '\0';
            while(strlen(Str) != 10)
            {
                memset(Str, '\0', STRING_MAX);
                printf("Invalid Number, it must be 10 digits long, please try again : ");
                fgets(Str, STRING_MAX, stdin);
                Str[strcspn(Str, "\n")] = '\0';
                i++;
                if(i == 5)
                {
                    printf("Too many attempts, you will be logged off\n");
                    return 0;
                }
            }
            sscanf(Str, "%10d", &Num);
            printf("Valid phone number !\n");

            //Binding statement
            sqlite3_bind_int(stmt, 1, Num);
            sqlite3_bind_int(stmt, 2, SuperId);

            //Execute the query
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                sqlite3_reset(stmt);
                sqlite3_finalize(stmt);
                return -1;
            }
            printf("Modification successful\n");
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            break;

            case 4 : //Exit

            printf("Hope to see you later !");
            return 0;
            break;

            case 5 : //Admin Employee info edit

            if(Admin == 1 || Admin == 2)
            {
                getchar();
                printf("Please enter the id of the employee whose informations you want to edit : ");
                fgets(Str, STRING_MAX, stdin);
                sscanf(Str, "%d", &ChoiceID);
                memset(Str, '\0', STRING_MAX);
                printf("Choose an option : ");
                printf("\n 1 - Edit Employee %d's Name \t 2 - Edit Employee %d's Age \t 3 - Edit Employee %d's Number \t 4 - Edit Employee %d's Position \t 5 - Edit Employee %d's Salary \t 6 - Exit"
                ,ChoiceID, ChoiceID, ChoiceID, ChoiceID, ChoiceID);
                fgets(Str, 2, stdin);
                sscanf(Str, "%1d", &Choice2);
                memset(Str, '\0', STRING_MAX);
                //Actual choice switch case for the admin
                switch(Choice2)
                {
                    case 1 : //Modify employee's name
                    
                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlEmpName0, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }
                    getchar();
                    printf("Provide the new full name : ");
                    fgets(Str, STRING_MAX, stdin);
                    strncpy(Name, Str,STRING_MAX);
                    Name[strcspn(Name, "\n")] = '\0';
                    memset(Str, '\0', STRING_MAX);

                    //Binding statement
                    sqlite3_bind_text(stmt, 1, Name, -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(stmt, 2, ChoiceID);
                    memset(Name, '\0', STRING_MAX);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;

                    case 2 : //Modify Employee's Age
                    
                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlEmpAge0, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }
                    getchar();
                    printf("Please enter the new age : ");
                    fgets(Str, STRING_MAX, stdin);
                    sscanf(Str, "%3d", &Age);
                    memset(Str, '\0', STRING_MAX);
                    //Valid Age Check
                    i = 0;
                    while(Age > 120 || Age < 0)
                    {
                        printf("Invalid Age, please try again : ");
                        fgets(Str, STRING_MAX, stdin);
                        sscanf(Str, "%3d", &Age);
                        memset(Str, '\0', STRING_MAX);
                        i++;
                        if(i == 5)
                        {
                            printf("Too many attempts, you will be logged off\n");
                            return 0;
                        }
                    }   

                    //Binding statement
                    sqlite3_bind_int(stmt, 1, Age);
                    sqlite3_bind_int(stmt, 2, ChoiceID);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;

                    case 3 : //Modify Employee's number

                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlEmpNum0, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }

                    //Phone number validation
                    i = 0;
                    getchar();
                    printf("Please provide the new Moroccan phone number : ");
                    fgets(Str, STRING_MAX, stdin);
                    Str[strcspn(Str, "\n")] = '\0';
                    while(strlen(Str) != 10)
                    {
                        memset(Str, '\0', STRING_MAX);
                        printf("Invalid Number, it must be 10 digits long, please try again : ");
                        fgets(Str, STRING_MAX, stdin);
                        Str[strcspn(Str, "\n")] = '\0';
                        i++;
                        if(i == 5)
                        {
                            printf("Too many attempts, you will be logged off\n");
                            return 0;
                        }
                    }
                    sscanf(Str, "%10d", &Num);
                    printf("Valid phone number !\n");

                    //Binding statement
                    sqlite3_bind_int(stmt, 1, Num);
                    sqlite3_bind_int(stmt, 2, ChoiceID);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;

                    case 4 : //Modify Employee's position

                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlEmpPosition1, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }
                    getchar();
                    printf("Provide the new position for the employee : ");
                    fgets(Str, 51, stdin);
                    strncpy(Position, Str,STRING_MAX);
                    Position[strcspn(Position, "\n")] = '\0';
                    memset(Str, '\0', STRING_MAX);

                    //Binding statement
                    sqlite3_bind_text(stmt, 1, Position, -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(stmt, 2, ChoiceID);
                    memset(Position, '\0', STRING_MAX);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;

                    case 5 : //Modify Employee's Salary

                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlEmpSalary1, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }
                    getchar();
                    printf("Please enter the new salary : ");
                    fgets(Str, STRING_MAX, stdin);
                    sscanf(Str, "%d", &Salary);
                    memset(Str, '\0', STRING_MAX);
                    //Valid Age Check
                    while(Salary > 9999999 || Salary < 0)
                    {
                        printf("Invalid Salary, please try again : ");
                        fgets(Str, STRING_MAX, stdin);
                        sscanf(Str, "%d", &Salary);
                        memset(Str, '\0', STRING_MAX);
                        i++;
                        if(i == 5)
                        {
                            printf("Too many attempts, you will be logged off\n");
                            return 0;
                        }
                    }   

                    //Binding statement
                    sqlite3_bind_int(stmt, 1, Salary);
                    sqlite3_bind_int(stmt, 2, ChoiceID);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;
                    
                    case 6 : //Exit

                    printf("Hope to see you later dear Admin");
                    return 0;
                    break;

                    default :
                    printf("Invalid choice, please try again");
                    Modify(db);
                }
            }
            else
            {
                printf("Invalid choice, please try again");
                Modify(db);
            }
            break;

            case 6 : //Edit a Student's info

            if(Admin == 1 || Admin == 2)
            {
                getchar();
                printf("Please enter the id of the student whose informations you want to edit : ");
                fgets(Str, STRING_MAX, stdin);
                sscanf(Str, "%d", &ChoiceID);
                memset(Str, '\0', STRING_MAX);
                printf("Choose an option : ");
                printf("\n 1 - Edit Student %d's Name \t 2 - Edit Student %d's Age \t 3 - Edit Student %d's Number \t 4 - Edit Student %d's Major \t 5 - Edit Student %d's Average Grade \t 6 - Exit"
                ,ChoiceID, ChoiceID, ChoiceID, ChoiceID, ChoiceID);
                fgets(Str, 2, stdin);
                sscanf(Str, "%1d", &Choice2);
                memset(Str, '\0', STRING_MAX);
                //Actual choice switch case for the admin
                switch(Choice2)
                {
                    case 1 : //Modify Student's name
                    
                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlStdName0, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }
                    getchar();
                    printf("Provide the new full name : ");
                    fgets(Str, STRING_MAX, stdin);
                    strncpy(Name, Str,STRING_MAX);
                    Name[strcspn(Name, "\n")] = '\0';
                    memset(Str, '\0', STRING_MAX);

                    //Binding statement
                    sqlite3_bind_text(stmt, 1, Name, -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(stmt, 2, ChoiceID);
                    memset(Name, '\0', STRING_MAX);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;

                    case 2 : //Modify Student's Age
                    
                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlStdAge0, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }
                    getchar();
                    printf("Please enter the new age : ");
                    fgets(Str, STRING_MAX, stdin);
                    sscanf(Str, "%3d", &Age);
                    memset(Str, '\0', STRING_MAX);
                    //Valid Age Check
                    i = 0;
                    while(Age > 120 || Age < 0)
                    {
                        printf("Invalid Age, please try again : ");
                        fgets(Str, STRING_MAX, stdin);
                        sscanf(Str, "%3d", &Age);
                        memset(Str, '\0', STRING_MAX);
                        i++;
                        if(i == 5)
                        {
                            printf("Too many attempts, you will be logged off\n");
                            return 0;
                        }
                    }   

                    //Binding statement
                    sqlite3_bind_int(stmt, 1, Age);
                    sqlite3_bind_int(stmt, 2, ChoiceID);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;

                    case 3 : //Modify Student's number

                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlStdNum0, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }

                    //Phone number validation
                    getchar();
                    printf("Please provide the new Moroccan phone number : ");
                    fgets(Str, STRING_MAX, stdin);
                    Str[strcspn(Str, "\n")] = '\0';
                    i = 0;
                    while(strlen(Str) != 10)
                    {
                        memset(Str, '\0', STRING_MAX);
                        printf("Invalid Number, it must be 10 digits long, please try again : ");
                        fgets(Str, STRING_MAX, stdin);
                        Str[strcspn(Str, "\n")] = '\0';
                        i++;
                        if(i == 5)
                        {
                            printf("Too many attempts, you will be logged off\n");
                            return 0;
                        }
                    }
                    sscanf(Str, "%10d", &Num);
                    printf("Valid phone number !\n");

                    //Binding statement
                    sqlite3_bind_int(stmt, 1, Num);
                    sqlite3_bind_int(stmt, 2, ChoiceID);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;

                    case 4 : //Modify Student's Major

                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlStdMajor1, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }
                    printf("Provide the new major for the student : ");
                    fgets(Str, 51, stdin);
                    strncpy(Major, Str,STRING_MAX);
                    Major[strcspn(Major, "\n")] = '\0';
                    memset(Str, '\0', STRING_MAX);

                    //Binding statement
                    sqlite3_bind_text(stmt, 1, Major, -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(stmt, 2, ChoiceID);
                    memset(Major, '\0', STRING_MAX);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;

                    case 5 : //Modify Student's AvgGrade

                    //Prepare statement
                    rc = sqlite3_prepare_v2(db, sqlStdAvgGrade1, -1, &stmt, NULL);
                    if(rc != SQLITE_OK)
                    {
                        fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                        return -1;
                    }
                    getchar();
                    printf("Please enter the new average grade : ");
                    fgets(Str, STRING_MAX, stdin);
                    sscanf(Str, "%f", &AvgGrade);
                    memset(Str, '\0', STRING_MAX);
                    //Valid Age Check
                    while(AvgGrade > 20 || AvgGrade < 0)
                    {
                        printf("Invalid Grade, please try again : ");
                        fgets(Str, STRING_MAX, stdin);
                        sscanf(Str, "%f", &AvgGrade);
                        memset(Str, '\0', STRING_MAX);
                        i++;
                        if(i == 5)
                        {
                            printf("Too many attempts, you will be logged off\n");
                            return 0;
                        }
                    }   

                    //Binding statement
                    sqlite3_bind_int(stmt, 1, AvgGrade);
                    sqlite3_bind_int(stmt, 2, ChoiceID);

                    //Execute the query
                    rc = sqlite3_step(stmt);
                    if(rc != SQLITE_DONE)
                    {
                        fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                        return -1;
                    }
                    printf("Modification successful\n");
                    sqlite3_reset(stmt);
                    sqlite3_finalize(stmt);
                    break;
                    
                    case 6 : //Exit

                    printf("Hope to see you later dear Admin");
                    return 0;
                    break;

                    default :
                    printf("Invalid choice, please try again");
                    Modify(db);
                }
            }
            else
            {
                printf("Invalid choice, please try again");
                Modify(db);
            }
            break;

            case 7 : //Edit credentials

            if(Admin == 2)
            {
                printf("What is the type of the user whose credentials you wanna modify ? (Student/Employee) : ");
                getchar();
                fgets(Str, STRING_MAX, stdin);
                Str[strcspn(Str, "\n")] = '\0';
                //Comparing the user input with the choice
                if(strcmp(Str, "Employee") == 0)
                {
                    memset(Str, '\0', STRING_MAX);
                    printf("Please enter the employee's id whose credentials you wanna modify : ");
                    fgets(Str, STRING_MAX, stdin);
                    sscanf(Str, "%d", &ChoiceID);
                    memset(Str, '\0', STRING_MAX);
                    printf("What info do you want to modify ?\n");
                    printf("1 - E-mail \t \t 2 - Password");
                    printf("Action : ");
                    fgets(Str, STRING_MAX, stdin);
                    sscanf(Str, "%1d", &Choice2);
                    memset(Str, '\0', STRING_MAX);

                    if(Choice2 == 1)
                    {
                        //Prepare statement
                        rc = sqlite3_prepare_v2(db, sqlEmpMail2, -1, &stmt, NULL);
                        if(rc != SQLITE_OK)
                        {
                            fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                            return -1;
                        }

                        //New E-mail intake
                        getchar();
                        printf("Enter the new E-mail :");
                        fgets(Str, STRING_MAX, stdin);
                        Str[strcspn(Str, "\n")] = '\0';
                        sscanf(Str,"%50[^@]@%50[^.].%50s",Add, Mail, Dom);
                        memset(Str, '\0',STRING_MAX);
                        //Valid e-mail verification
                        while (!((strcmp(Mail, "gmail") == 0) || (strcmp(Mail, "outlook") == 0) || (strcmp(Mail, "yahoo") == 0)) || 
                            !((strcmp(Dom, "com") == 0) || (strcmp(Dom, "org") == 0) || (strcmp(Dom, "ma") == 0) || (strcmp(Dom, "fr") == 0)))
                        {
                            getchar();
                            printf("Invalid mail provider or domain, please try again: ");
                            fgets(Str, STRING_MAX, stdin);
                            sscanf(Str, "%50[^@]@%50[^.].%50s", Add, Mail, Dom);  
                            memset(Str, '\0', STRING_MAX);
                        }
                        //Concatenating the parts of the e-mail address into one string
                        strcat(Add, "@");
                        strcat(Add, Mail);
                        strcat(Add, ".");
                        strcat(Add, Dom);
                        free(Mail);
                        free(Dom);
                        printf("Valid E-mail address");
                        //Binding statement
                        sqlite3_bind_text(stmt, 1, Add, -1, SQLITE_TRANSIENT);
                        sqlite3_bind_int(stmt, 2, ChoiceID);

                        //Execute the query
                        rc = sqlite3_step(stmt);
                        if(rc != SQLITE_DONE)
                        {
                            fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                            sqlite3_reset(stmt);
                            sqlite3_finalize(stmt);
                            return -1;
                        }
                        printf("Modification successful\n");
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                    }
                    else if(Choice2 == 2)
                    {
                        //Prepare statement
                        rc = sqlite3_prepare_v2(db, sqlEmpPass2, -1, &stmt, NULL);
                        if(rc != SQLITE_OK)
                        {
                            fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                            return -1;
                        }

                        //New Password intake
                        printf("Enter the new password : ");
                        fgets(Str, STRING_MAX, stdin);
                        strncpy(Key, Str, STRING_MAX);
                        Key[strcspn(Key, "\n")] = '\0';
                        memset(Str, '\0', STRING_MAX);
                        //Short passwords verfication
                        while(strlen(Key) < 8)
                        {
                            printf("Password too short, must at least contain 8 characters\n");
                            printf("Enter a new password that is at least 8 characters long : ");
                            fgets(Str, STRING_MAX, stdin);
                            strncpy(Key, Str, STRING_MAX);
                            Key[strcspn(Key, "\n")] = '\0';
                            memset(Str, '\0', STRING_MAX);
                        }
                        printf("Re-enter your password to verify it : ");
                        fgets(Str, STRING_MAX, stdin);
                        strncpy(Key2, Str, STRING_MAX);
                        Key2[strcspn(Key2, "\n")] = '\0';
                        memset(Str, '\0', STRING_MAX);
                        //First && Second Password matching verification
                        while(strcmp(Key, Key2) != 0)
                        {
                            printf("The first password doesnt match the second one, please re-enter the second password : ");
                            fgets(Str, STRING_MAX, stdin);
                            strncpy(Key2, Str, STRING_MAX);
                            Key2[strcspn(Key2, "\n")] = '\0';
                            memset(Str, '\0', STRING_MAX);
                        }
                        memset(Key2, '\0', STRING_MAX);

                        //Generating the salt
                        rc = bcrypt_gensalt(12, salt);
                        if(rc != 0)
                        {
                            fprintf(stderr, "Error while generating the salt : %d\n", rc);
                            return -1;
                        }
                        //Hashing the password
                        rc = bcrypt_hashpw(Key, salt, Key2);
                        if(rc != 0)
                        {
                            fprintf(stderr, "Error while hashing the password : %d\n", rc);
                            return -1;
                        }
                        //Binding statement
                        sqlite3_bind_text(stmt, 1, Key2, -1, SQLITE_TRANSIENT);
                        sqlite3_bind_int(stmt, 2, ChoiceID);
                        memset(Key2, '\0', STRING_MAX);

                        //Execute the query
                        rc = sqlite3_step(stmt);
                        if(rc != SQLITE_DONE)
                        {
                            fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                            sqlite3_reset(stmt);
                            sqlite3_finalize(stmt);
                            return -1;
                        }
                        printf("Modification successful\n");
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                    }
                    else
                    {
                        printf("Invalid choice, please try again");
                        Modify(db);
                    }
                    return 1;
                }
                if(strcmp(Str, "Student") == 0)
                {
                    getchar();
                    memset(Str, '\0', STRING_MAX);
                    printf("Please enter the student's id whose credentials you wanna modify : ");
                    fgets(Str, STRING_MAX, stdin);
                    sscanf(Str, "%d", &ChoiceID);
                    memset(Str, '\0', STRING_MAX);
                    getchar();
                    printf("What info do you want to modify ?\n");
                    printf("1 - E-mail \t \t 2 - Password");
                    fgets(Str, STRING_MAX, stdin);
                    sscanf(Str, "%1d", &Choice2);
                    memset(Str, '\0', STRING_MAX);

                    if(Choice2 == 1)
                    {
                        //Prepare statement
                        rc = sqlite3_prepare_v2(db, sqlStdMail2, -1, &stmt, NULL);
                        if(rc != SQLITE_OK)
                        {
                            fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                            return -1;
                        }

                        //New E-mail intake
                        getchar();
                        printf("Enter the new E-mail");
                        fgets(Str, STRING_MAX, stdin);
                        Str[strcspn(Str, "\n")] = '\0';
                        sscanf(Str,"%50[^@]@%50[^.].%50s",Add, Mail, Dom);
                        memset(Str, '\0',STRING_MAX);
                        //Valid e-mail verification
                        while (!((strcmp(Mail, "gmail") == 0) || (strcmp(Mail, "outlook") == 0) || (strcmp(Mail, "yahoo") == 0)) || 
                            !((strcmp(Dom, "com") == 0) || (strcmp(Dom, "org") == 0) || (strcmp(Dom, "ma") == 0) || (strcmp(Dom, "fr") == 0)))
                        {
                            getchar();
                            printf("Invalid mail provider or domain, please try again: ");
                            fgets(Str, STRING_MAX, stdin);
                            sscanf(Str, "%50[^@]@%50[^.].%50s", Add, Mail, Dom);  
                            memset(Str, '\0', STRING_MAX);
                        }
                        //Concatenating the parts of the e-mail address into one string
                        strcat(Add, "@");
                        strcat(Add, Mail);
                        strcat(Add, ".");
                        strcat(Add, Dom);
                        free(Mail);
                        free(Dom);
                        printf("Valid E-mail address");
                        //Binding statement
                        sqlite3_bind_text(stmt, 1, Add, -1, SQLITE_TRANSIENT);
                        sqlite3_bind_int(stmt, 2, ChoiceID);

                        //Execute the query
                        rc = sqlite3_step(stmt);
                        if(rc != SQLITE_DONE)
                        {
                            fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                            sqlite3_reset(stmt);
                            sqlite3_finalize(stmt);
                            return -1;
                        }
                        printf("Modification successful\n");
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                    }
                    else if(Choice2 == 2)
                    {
                        //Prepare statement
                        rc = sqlite3_prepare_v2(db, sqlStdPass2, -1, &stmt, NULL);
                        if(rc != SQLITE_OK)
                        {
                            fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                            return -1;
                        }

                        //New Password intake
                        getchar();
                        printf("Enter the new password");
                        fgets(Str, STRING_MAX, stdin);
                        strncpy(Key, Str, STRING_MAX);
                        Key[strcspn(Key, "\n")] = '\0';
                        memset(Str, '\0', STRING_MAX);
                        //Short passwords verfication
                        while(strlen(Key) < 8)
                        {
                            getchar();
                            printf("Password too short, must at least contain 8 characters\n");
                            printf("Enter a new password that is at least 8 characters long : ");
                            fgets(Str, STRING_MAX, stdin);
                            strncpy(Key, Str, STRING_MAX);
                            Key[strcspn(Key, "\n")] = '\0';
                            memset(Str, '\0', STRING_MAX);
                        }
                        getchar();
                        printf("Re-enter your password to verify it : ");
                        fgets(Str, STRING_MAX, stdin);
                        strncpy(Key2, Str, STRING_MAX);
                        Key2[strcspn(Key2, "\n")] = '\0';
                        memset(Str, '\0', STRING_MAX);
                        //First && Second Password matching verification
                        while(strcmp(Key, Key2) != 0)
                        {
                            getchar();
                            printf("The first password doesnt match the second one, please re-enter the second password : ");
                            fgets(Str, STRING_MAX, stdin);
                            strncpy(Key2, Str, STRING_MAX);
                            Key2[strcspn(Key2, "\n")] = '\0';
                            memset(Str, '\0', STRING_MAX);
                        }
                        memset(Key2, '\0', STRING_MAX);

                        //Generating the salt
                        rc = bcrypt_gensalt(12, salt);
                        if(rc != 0)
                        {
                            fprintf(stderr, "Error while generating the salt : %d\n", rc);
                            return -1;
                        }
                        //Hashing the password
                        rc = bcrypt_hashpw(Key, salt, Key2);
                        if(rc != 0)
                        {
                            fprintf(stderr, "Error while hashing the password : %d\n", rc);
                            return -1;
                        }
                        //Binding statement
                        sqlite3_bind_text(stmt, 1, Key2, -1, SQLITE_TRANSIENT);
                        sqlite3_bind_int(stmt, 2, ChoiceID);
                        memset(Key2, '\0', STRING_MAX);

                        //Execute the query
                        rc = sqlite3_step(stmt);
                        if(rc != SQLITE_DONE)
                        {
                            fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                            sqlite3_reset(stmt);
                            sqlite3_finalize(stmt);
                            return -1;
                        }
                        printf("Modification successful\n");
                        sqlite3_reset(stmt);
                        sqlite3_finalize(stmt);
                    }
                    else
                    {
                        printf("Invalid choice, please try again");
                        Modify(db);
                    }
                    return 1;
                }
            }
            else
            {
                printf("Invalid choice, please try again");
                Modify(db);
            }
        }
        return 1;
    }
    else if(Is_Student == 1) //For students only
    {    //Printing the different options
        printf("Choose an option : ");
        printf("\n 1 - Edit Name \t 2 - Edit Age \t 3 - Edit Number\t4 - Exit");
        printf("\n Option : ");
        fgets(Str, 2, stdin);
        sscanf(Str, "%1d", &Choice);
        memset(Str, '\0', STRING_MAX);
        
        //Actual choice switch case
        switch(Choice)
        {
            case 1 : //Modify name
            
            //Prepare statement
            rc = sqlite3_prepare_v2(db, sqlStdName0, -1, &stmt, NULL);
            if(rc != SQLITE_OK)
            {
                fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                return -1;
            }
            getchar();
            printf("Provide your full name : ");
            fgets(Str, STRING_MAX, stdin);
            strncpy(Name, Str,STRING_MAX);
            Name[strcspn(Name, "\n")] = '\0';
            memset(Str, '\0', STRING_MAX);

            //Binding statement
            sqlite3_bind_text(stmt, 1, Name, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, SuperId);

            //Execute the query
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                sqlite3_reset(stmt);
                sqlite3_finalize(stmt);
                return -1;
            }
            printf("Modification successful\n");
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            break;

            case 2 : //Modify Age
            
            //Prepare statement
            rc = sqlite3_prepare_v2(db, sqlStdAge0, -1, &stmt, NULL);
            if(rc != SQLITE_OK)
            {
                fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                return -1;
            }
            getchar();
            printf("Please enter your age : ");
            fgets(Str, STRING_MAX, stdin);
            sscanf(Str, "%3d", &Age);
            memset(Str, '\0', STRING_MAX);
            //Valid Age Check
            i = 0;
            while(Age > 120 || Age < 0)
            {
                printf("Invalid Age, please try again : ");
                fgets(Str, STRING_MAX, stdin);
                sscanf(Str, "%3d", &Age);
                memset(Str, '\0', STRING_MAX);
                i++;
                if(i == 5)
                {
                    printf("Too many attempts, you will be logged off\n");
                    return 0;
                }
            }   

            //Binding statement
            sqlite3_bind_int(stmt, 1, Age);
            sqlite3_bind_int(stmt, 2, SuperId);

            //Execute the query
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                sqlite3_reset(stmt);
                sqlite3_finalize(stmt);
                return -1;
            }
            printf("Modification successful\n");
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            break;

            case 3 : //Modify number

            //Prepare statement
            rc = sqlite3_prepare_v2(db, sqlStdNum0, -1, &stmt, NULL);
            if(rc != SQLITE_OK)
            {
                fprintf(stderr, "Error while preparing the statement : %d\n", rc);
                return -1;
            }

            //Phone number validation
            getchar();
            printf("Please provide your Moroccan phone number : ");
            fgets(Str, STRING_MAX, stdin);
            Str[strcspn(Str, "\n")] = '\0';
            while(strlen(Str) != 10)
            {
                memset(Str, '\0', STRING_MAX);
                printf("Invalid Number, it must be 10 digits long, please try again : ");
                fgets(Str, STRING_MAX, stdin);
                Str[strcspn(Str, "\n")] = '\0';
                i++;
                if(i == 5)
                {
                    printf("Too many attempts, you will be logged off\n");
                    return 0;
                }
            }
            sscanf(Str, "%10d", &Num);
            printf("Valid phone number !\n");

            //Binding statement
            sqlite3_bind_int(stmt, 1, Num);
            sqlite3_bind_int(stmt, 2, SuperId);

            //Execute the query
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_DONE)
            {
                fprintf(stderr, "Error while executing the statement 2 : %s\n", sqlite3_errmsg(db));
                sqlite3_reset(stmt);
                sqlite3_finalize(stmt);
                return -1;
            }
            printf("Modification successful\n");
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            break;

            case 4 : //Exit

            printf("Hope to see you later !\n");
            return 0;
            break; 
        }
     return 1;
    }
    else
    {
        printf("An error has occured");
        return -1;
    } 
}

int emp_callback(void *data, int columnCount, char **columnValues, char **columnNames) 
{
    (void)data;
    printf("\nEmployees Record : \n");
    for (int i = 0; i < columnCount; i++) {
        printf("%s = %s\n", columnNames[i], columnValues[i] ? columnValues[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int std_callback(void *data, int columnCount, char **columnValues, char **columnNames) 
{
    (void)data;
    printf("\nStudents Record : \n");
    for (int i = 0; i < columnCount; i++) {
        printf("%s = %s\n", columnNames[i], columnValues[i] ? columnValues[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int credentials_callback(void *data, int columnCount, char **columnValues, char **columnNames) 
{
    (void)data;
    printf("\nCredentials : \n");
    for (int i = 0; i < columnCount; i++) {
        printf("%s = %s\n", columnNames[i], columnValues[i] ? columnValues[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int Show(sqlite3* db)
{
    char* Str = (char*)malloc(STRING_MAX * sizeof(char));
    char* Name = (char*)malloc(STRING_MAX * sizeof(char));
    char* Position = (char*)malloc(STRING_MAX * sizeof(char));
    char* Major = (char*)malloc(STRING_MAX * sizeof(char));
    char* Admin = (char*)malloc(STRING_MAX * sizeof(char));\
    char* errMsg = NULL;
    int Age, Num, Salary, rc, Choice;
    int AdminCheck = 0;
    float AvgGrade;
    const char* sqlCheck = "SELECT Admin FROM Employees WHERE ID = ?;";
    const char* sqlShowStd = "SELECT * FROM Students WHERE StdID = ?;";
    const char* sqlShowEmp = "SELECT * FROM Employees WHERE ID = ?;";
    const char* sqlShowAllStd = "SELECT StdID AS 'ID', StdName AS 'Name', StdAge AS 'Age', StdMajor AS 'Major', StdNum AS 'Phone Number', AvgGrade AS 'Average Grade' FROM Students;";
    const char* sqlShowAllEmp = "SELECT ID AS 'ID', EmpName AS 'Name', EmpAge AS 'Age', EmpPosition AS 'Position', EmpNum AS 'Phone Number', Salary FROM Employees;";
    const char* sqlShowCredentials = "SELECT * FROM Credentials;";
    sqlite3_stmt* stmt;
    sqlite3_stmt* stmtCheck;

    if(Is_Student == 0)
    {
        //Preparing the query
        rc = sqlite3_prepare_v2(db, sqlCheck, -1, &stmtCheck, NULL);
        if(rc != SQLITE_OK)
        {
            fprintf(stderr, "Error while preparing the statement : %d\n", rc);
            return -1;
        }
        //Binding the statement
        sqlite3_bind_int(stmtCheck, 1, SuperId);

        //Executing the statement
        rc = sqlite3_step(stmtCheck);
        if(rc == SQLITE_ROW) 
        {
        //Retrives the Admin status
        strncpy(Str, (const char*) sqlite3_column_text(stmtCheck, 0), STRING_MAX);
        Str[strcspn(Str, "\n")] = '\0';
        sqlite3_finalize(stmtCheck);
        }
        else if(rc == SQLITE_DONE) 
        {
        printf("No valid entry found");
        sqlite3_finalize(stmtCheck);
        return 0;
        }
        else
        {
        fprintf(stderr, "Query execution failed : %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmtCheck);
        return -1;
        }
        //Checks if super admin or just normal admin
        if(strcmp(Str, "Yes") == 0)
        {
            AdminCheck= 1;
        }
        else if(strcmp(Str, "Super") == 0)
        {
            AdminCheck = 2;
        }
        memset(Str, '\0', STRING_MAX);
    }
    
    if(Is_Student == 1)
    {
        //Preparing the statement
        rc = sqlite3_prepare_v2(db, sqlShowStd, -1, &stmt, NULL);
        if(rc != SQLITE_OK)
        {
            fprintf(stderr, "Error while preparing statement : %s\n", sqlite3_errmsg(db));
            return -1;
        }

        //Binding the parameter
        sqlite3_bind_int(stmt, 1, SuperId);

        //Executing the query
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
            strncpy(Name, (const char*) sqlite3_column_text(stmt, 1), STRING_MAX);
            Age = sqlite3_column_int(stmt, 2);
            strncpy(Major, (const char*) sqlite3_column_text(stmt, 3), STRING_MAX);
            Num = sqlite3_column_int(stmt, 4);
            AvgGrade = sqlite3_column_int(stmt, 5);
            printf("Student with ID : %d's record : \n Name = %s \n Age = %d \n Major = %s \n Phone Number = %10d \n Average Grade = %f"
            , SuperId, Name, Age, Major, Num, AvgGrade);
            sqlite3_finalize(stmt);
            return 1;
        }
        else if(rc == SQLITE_DONE)
        {
            printf("No match found");
            sqlite3_finalize(stmt);
            return 0;
        }
        else
        {
            fprintf(stderr, "Error while executing the query : %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }
    else if(Is_Student == 0 && AdminCheck == 0)
    {
        //Preparing the statement
        rc = sqlite3_prepare_v2(db, sqlShowEmp, -1, &stmt, NULL);
        if(rc != SQLITE_OK)
        {
            fprintf(stderr, "Error while preparing statement : %s\n", sqlite3_errmsg(db));
            return -1;
        }

        //Binding the parameter
        sqlite3_bind_int(stmt, 1, SuperId);

        //Executing the query
        rc = sqlite3_step(stmt);
        if(rc == SQLITE_ROW)
        {
            strncpy(Name, (const char*) sqlite3_column_text(stmt, 1), STRING_MAX);
            Age = sqlite3_column_int(stmt, 2);
            strncpy(Position, (const char*) sqlite3_column_text(stmt, 3), STRING_MAX);
            Num = sqlite3_column_int(stmt, 4);
            Salary = sqlite3_column_int(stmt, 5);
            strncpy(Admin, (const char*) sqlite3_column_text(stmt, 6), STRING_MAX);
            printf("Employee with ID = %d's record : \n Name = %s \n Age = %d \n Position = %s \n Phone Number = %10d \n Salary = %d \n Admin = %s \n"
            , SuperId, Name, Age, Position, Num, Salary, Admin);
            sqlite3_finalize(stmt);
            return 1;
        }
        else if(rc == SQLITE_DONE)
        {
            printf("No match found");
            sqlite3_finalize(stmt);
            return 0;
        }
        else
        {
            fprintf(stderr, "Error while executing the query : %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return -1;
        }
    }
    else if(Is_Student == 0 && AdminCheck == 1)
    {
        printf("Dear Admin, what data do you want to see ? \n");
        printf("1 - Personal Data \t 2 - Student's Data \t 3 - Employee's Data\n");
        printf("Action : ");
        getchar();
        memset(Str, '\0', STRING_MAX);
        fgets(Str, 2, stdin);
        sscanf(Str, "%d", &Choice);

        switch(Choice)
        {
            case 1 : //Personal data

            //Preparing the statement
            rc = sqlite3_prepare_v2(db, sqlShowEmp, -1, &stmt, NULL);
            if(rc != SQLITE_OK)
            {
                fprintf(stderr, "Error while preparing statement : %s\n", sqlite3_errmsg(db));
                return -1;
            }

            //Binding the parameter
            sqlite3_bind_int(stmt, 1, SuperId);

            //Executing the query
            rc = sqlite3_step(stmt);
            if(rc == SQLITE_ROW)
            {
                strncpy(Name, (const char*) sqlite3_column_text(stmt, 1), STRING_MAX);
                Age = sqlite3_column_int(stmt, 2);
                strncpy(Position, (const char*) sqlite3_column_text(stmt, 3), STRING_MAX);
                Num = sqlite3_column_int(stmt, 4);
                Salary = sqlite3_column_int(stmt, 5);
                strncpy(Admin, (const char*) sqlite3_column_text(stmt, 6), STRING_MAX);
                printf("Employee with ID = %d's record : \n Name : %s \n Age : %d \n Position : %s \n Phone Number : %10d \n Salary : %d \n Admin : %s \n"
                , SuperId, Name, Age, Position, Num, Salary, Admin);
                sqlite3_finalize(stmt);
                return 1;
            }
            else if(rc == SQLITE_DONE)
            {
                printf("No match found");
                sqlite3_finalize(stmt);
                return 0;
            }
            else
            {
                fprintf(stderr, "Error while executing the query : %s\n", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return -1;
            }
            break;

            case 2 : //Students data

            sqlite3_exec(db, sqlShowAllStd, std_callback, NULL, &errMsg);
            break;

            case 3 : //Employees data

            sqlite3_exec(db, sqlShowAllEmp, emp_callback, NULL, &errMsg);
            break;
        }

    }
    else if(Is_Student == 0 && AdminCheck == 2)
    {
                printf("Super Admin ! what data do you want to see ? \n");
        printf("1 - Personal Data \t 2 - Student's Data \t 3 - Employee's Data \t 4 - Credentials \n");
        printf("Action : ");
        getchar();
        memset(Str, '\0', STRING_MAX);
        fgets(Str, 2, stdin);
        sscanf(Str, "%d", &Choice);

        switch(Choice)
        {
            case 1 : //Personal data

            //Preparing the statement
            rc = sqlite3_prepare_v2(db, sqlShowEmp, -1, &stmt, NULL);
            if(rc != SQLITE_OK)
            {
                fprintf(stderr, "Error while preparing statement : %s\n", sqlite3_errmsg(db));
                return -1;
            }

            //Binding the parameter
            sqlite3_bind_int(stmt, 1, SuperId);

            //Executing the query
            rc = sqlite3_step(stmt);
            if(rc == SQLITE_ROW)
            {
                strncpy(Name, (const char*) sqlite3_column_text(stmt, 1), STRING_MAX);
                Age = sqlite3_column_int(stmt, 2);
                strncpy(Position, (const char*) sqlite3_column_text(stmt, 3), STRING_MAX);
                Num = sqlite3_column_int(stmt, 4);
                Salary = sqlite3_column_int(stmt, 5);
                strncpy(Admin, (const char*) sqlite3_column_text(stmt, 6), STRING_MAX);
                printf("Employee with ID = %d's record : \n Name : %s \n Age : %d \n Position : %s \n Phone Number : %10d \n Salary : %d \n Admin : %s \n"
                , SuperId, Name, Age, Position, Num, Salary, Admin);
                sqlite3_finalize(stmt);
                return 1;
            }
            else if(rc == SQLITE_DONE)
            {
                printf("No match found");
                sqlite3_finalize(stmt);
                return 0;
            }
            else
            {
                fprintf(stderr, "Error while executing the query : %s\n", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return -1;
            }
            break;

            case 2 : //Students data

            sqlite3_exec(db, sqlShowAllStd, std_callback, NULL, &errMsg);
            break;

            case 3 : //Employees data

            sqlite3_exec(db, sqlShowAllEmp, emp_callback, NULL, &errMsg);
            break;

            case 4 : //Credentials

            sqlite3_exec(db, sqlShowCredentials, credentials_callback, NULL, &errMsg);
            break;
        }
    }
    else
    {
        printf("An error has occured");
        return -1;
    }
    return 0;
}

int Login(sqlite3* db)
{
    char* Mail = (char*)malloc(STRING_MAX * sizeof(char));
    char* Key = (char*)malloc(STRING_MAX * sizeof(char));
    char* Str = (char*)malloc(STRING_MAX * sizeof(char));
    char* Stored_hash = (char*)malloc(STRING_MAX * sizeof(char));
    const char* sql = "SELECT Pass FROM Credentials WHERE Mail = ?;";
    sqlite3_stmt* stmt;
    int rc;

    printf("Welcome ! Please enter your e-mail address : ");
    getchar();
    fgets(Str, STRING_MAX, stdin);
    strncpy(Mail, Str, STRING_MAX);
    Mail[strcspn(Mail, "\n")] = '\0';
    memset(Str, '\0', STRING_MAX);
    
    //Prepare the SQLITE Statement
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if(rc != SQLITE_OK)
    {
        fprintf(stderr, "Error while preparing the statement : %s\n", sqlite3_errmsg(db));
        return -1;
    }
    //Binding the mail parameter
    sqlite3_bind_text(stmt, 1, Mail, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if(rc == SQLITE_ROW) 
    {
        strncpy(Stored_hash, (const char*) sqlite3_column_text(stmt, 0), STRING_MAX);
        printf("Valid e-mail !\n");
        sqlite3_finalize(stmt);
    }
    else if(rc == SQLITE_DONE) 
    {
        printf("No e-mail found\n");
        sqlite3_finalize(stmt);
        return 0;
    }
    else
    {
        fprintf(stderr, "Query execution failed : %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    //Entering your password
    printf("Please enter your password : ");
    fgets(Str, STRING_MAX, stdin);
    strncpy(Key, Str, STRING_MAX);
    Key[strcspn(Key, "\n")] = '\0';
    memset(Str, '\0', STRING_MAX);
    //Password Brute Force Timeout
    for(int i = 0; i < 5; i++)
    {
        //Checking if the password against the hashed one
        if(bcrypt_checkpw(Key, Stored_hash) == 0)
        {
            printf("Password entered is correct \n");
            SuperId = GetID(db, Mail);
            free(Str);
            free(Mail);
            free(Key);
            free(Stored_hash);
            Is_Logged = 1;
            return 1;
        }
        else
        {
            printf("Incorrect password entered, please try again : ");
            fgets(Str, STRING_MAX, stdin);
            strncpy(Key, Str, STRING_MAX);
            Key[strcspn(Key, "\n")] = '\0';
            memset(Str, '\0', STRING_MAX);
        }
    }

    printf("Too many attempts, you will be logged off");
    free(Str);
    free(Mail);
    free(Key);
    free(Stored_hash);
    return 0;
}

int main(void)
{
    sqlite3* db;
    int rc;
    char* Str = (char*)malloc(STRING_MAX * sizeof(char));
    int Choice;

    rc = sqlite3_open("schooldb.sqlite3", &db);
    if(rc != SQLITE_OK)
    {
        fprintf(stderr, "Error while opening db : %s\n", sqlite3_errmsg(db));
        return 1;
    }
    //Employee / Student distinction
    printf("Do you want to login/register as a student or as an employee ? (Student/Employee) : ");
    fgets(Str, STRING_MAX, stdin);
    Str[strcspn(Str, "\n")] = '\0';
    if(strcmp(Str, "Student") == 0) Is_Student = 1;
    else if(strcmp(Str, "Employee") == 0) Is_Student = 0;
    else {printf("Invalid choice, please try again"); return 0;}

    printf("Welcome dear current/future %s what action do you want to undertake ? :\n", Str);
    printf("\n\n 1 - Login \t2 - Register \t 3 - Exit\n");
    printf("\n \t\tAction : ");
    memset(Str, '\0', STRING_MAX);
    fgets(Str, 2, stdin);
    sscanf(Str, "%d", &Choice);
    switch(Choice)
    {
        case 1 :
        memset(Str, '\0',STRING_MAX);
        if(Login(db) != 1)
        {
            printf("Login has failed do you want to try again ? (Y/N) : ");
            fgets(Str, 2, stdin);
            Str[1] = '\0';

            if(strcmp(Str, "Y") == 0) Login(db);
            else
            {
            printf("See you next time !");
            return 0;
            }
        }
        break;

        case 2 :
        memset(Str, '\0',STRING_MAX);
        if(Register(db) == 1)
        {
            printf("Do you want to login ? (Y/N ): ");
            fgets(Str, 2, stdin);
            Str[1] = '\0';
            if(strcmp(Str, "Y") == 0) Login(db);
            else
            {
                printf("See you next time then !\n");
                return 0;
            }

        }
        else
        {
            printf("Registering has failed do you want to try again ? (Y/N) : ");
            fgets(Str, 2, stdin);
            Str[1] = '\0';

            if(strcmp(Str, "Y") == 0) Register(db);
            else
            {
            printf("See you next time !\n");
            return 0;
            } 
        }
        break;

        case 3 :
        memset(Str, '\0',STRING_MAX);
        printf("See you later !");
        return 0;
        break;

        default :
        printf("Invalid choice, you will be logged off");
    }

    if(Is_Logged == 1)
    {
        printf("What action do you want to undertake ? : \n");
        printf("1 - Show infos \t 2 - Edit Data\n");
        printf("Action : ");
        memset(Str, '\0', STRING_MAX);
        fgets(Str, 2, stdin);
        sscanf(Str, "%d", &Choice);
        if(Choice == 1) Show(db);
        else if(Choice == 2) Modify(db); 
    }
    return 1;
}
