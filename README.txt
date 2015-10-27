AUTHER: MAX FALK NILLSSON
DATE: 2015-10-26

ABOUT:
Uses SQLITE as the database. 
Communicates with tcp and allows multiple clients.
Server sends and recieves questions from client which it executes on the database, and sends the result back to the client.


INSTALL:
1. Unpack anywhere.
2. Navigate to the folder "exe" for executables for the client and the database.
3. Navigate to the folder "code" for the source code.

HOW TO USE:
Start Server first then the clients.
The clients has 3 fields, a listview were the reult of a query will be presented.
One edittext field were queries can be written and a send button to send the written query to the server.
Queries are written with SQL and most features that sqlite supports should work.

EXAMPLES:
select * from data , will list all the data from the database.
select * from data where number like "TM%" , will list all rows where the field number start with TM.
select * from data where number like "TM%" and digits = 8 , same as before but digits is equal to 8 also.
select date, number, digits  from data where digits = 8 , will only show data for date, number, digits  on the rows were digits is equal to 8.
