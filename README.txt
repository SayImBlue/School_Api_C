The code is supposed to with an SQLITE database, here's the script for it and the tables :

CREATE TABLE Students
(
 StdID INT PRIMARY KEY NOT NULL,
 StdName TEXT NOT NULL,
 StdAge INT NOT NULL,
 StdMajor CHAR(50) NOT NULL,
 StdNum SMALLINT,
 AvgGrade DECIMAL(2,2)
);

CREATE TABLE Employees 
(
 ID INT PRIMARY KEY NOT NULL,
 EmpName TEXT NOT NULL,
 EmpAge INT NOT NULL,
 EmpPosition CHAR(50) NOT NULL,
 EmpNum SMALLINT NOT NULL,
 Salary INT,
 Admin CHAR(5) NOT NULL
);

CREATE TABLE Credentials
(
 ID INT UNIQUE,
 StdID INT UNIQUE,
 Mail TEXT NOT NULL UNIQUE,
 Pass TEXT NOT NULL,
 CONSTRAINT check_ID_or_StdID_not_null CHECK (ID IS NOT NULL OR StdID IS NOT NULL)
);

NOTE : I also use Bcrypt in my code
