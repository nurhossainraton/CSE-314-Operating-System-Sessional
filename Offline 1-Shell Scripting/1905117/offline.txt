#!/bin/bash
submissionfolder=$1
targetfolder=$2
testfolder=$3
answerfolder=$4


if [ $# == 5 ] || [ $# == 4 ];
   then
   bool=1
   
elif [ $# -lt 4 ] || [ $# -gt 6 ] || [ $5 != "-v" ] || [ $6 != "-noexecute" ] ;
then
    echo Usage:
    echo "./organize.sh <submission folder> <target folder> <test folder> <answer folder> [-v] [-noexecute]"
    echo -v: verbose
    echo -noexecute: do not execute code files
    showmsg=0
    kill -INT $$
    
fi
if [ $5 = "-v" ];
    then
           file_count=$(ls -p $testfolder | grep -v / | wc -l) 
           echo Found $file_count files   
           showmsg=1
    fi
if [ $6 = "-noexecute" ];
then
    bool=0
fi
mkdir -p $targetfolder
touch targets/result.csv 
csv_file="$targetfolder/result.csv"
chmod +w "$targetfolder/result.csv"
echo "Student_id,type,matched,not_matched" > "$csv_file"

bool=0
showmsg=0


fileexecute()
{
   studentid=$1
   foldername=$2
   
   if [ showmsg == 1 ];
   then
   echo Organizing files of $studentid
   fi
   
  
   if [ $foldername = "C" ];
   then
   
   matched=0
   unmatched=0
   
   
   for i in "$testfolder"/test*.txt
   do 
   number=${i: 10: 1}
   
   #echo number is  $number
   gcc $targetfolder/C/$studentid/main.c -o $targetfolder/C/$studentid/main.out
   $targetfolder/C/$studentid/main.out < $i > $targetfolder/C/$studentid/out$number.txt
   diffresult=$(diff $answerfolder/ans$number.txt $targetfolder/C/$studentid/out$number.txt)
   
    if [ -n "$diffresult" ]; then
    unmatched=$((unmatched+1))
    else
    matched=$((matched+1))
    fi
   
   done
   echo "$studentid,C,$matched,$unmatched" >> "$csv_file"
   
   elif [ $foldername = "Python" ];
   then
   matched=0
   unmatched=0
   #mkdir -p targets/Python/$studentid
   
   for i in "$testfolder"/test*.txt
   do 
   number=${i: 10: 1}
   
   python3 $targetfolder/Python/$studentid/main.py < $i > $targetfolder/Python/$studentid/out$number.txt
   diffresult=$(diff $answerfolder/ans$number.txt $targetfolder/Python/$studentid/out$number.txt)
   
    if [ -n "$diffresult" ]; then
    unmatched=$((unmatched+1))
    else
    matched=$((matched+1))
    fi
   
   done
    echo "$studentid,Python,$matched,$unmatched" >> "$csv_file"
   
   elif [ $foldername = "Java" ];
   then
   matched=0
   unmatched=0
   #mkdir -p targets/Java/$studentid 
  
   for i in "$testfolder"/test*.txt
   do 
   number=${i: 10: 1}
   javac $targetfolder/Java/$studentid/Main.java 
   java -cp $targetfolder/Java/$studentid Main < $i > $targetfolder/Java/$studentid/out$number.txt
   
    diffresult=$(diff $answerfolder/ans$number.txt $targetfolder/Java/$studentid/out$number.txt)
   
    if [ -n "$diffresult" ]; then
    unmatched=$((unmatched+1))
    else
    matched=$((matched+1))
    fi
   
   done
    echo "$studentid,Java,$matched,$unmatched" >> "$csv_file"
   fi
}

searchextension()
{
   studentid=$1
   foldername=$3
   filename=$2
   
   
   mkdir -p targets/$foldername/$studentid
   if [ $foldername = "C" ];
   then
   cp $filename targets/C/$studentid/main.c
   
   elif [ $foldername = "Python" ];
   then
   cp $filename targets/Python/$studentid/main.py
   
   elif [ $foldername = "Java" ];
   then
   cp $filename targets/Java/$studentid/Main.java
   
   fi
   
    if [ $showmsg == 1 ];
    then
    echo Organizing files of $studentid
   fi
   
   if [ $bool == 1 ];
then
    echo Executing files of $studentid
    fileexecute $studentid $foldername
   
   fi

   
   
   
   
}

for zipfile in "${submissionfolder}"/*.zip
do 
  filename=""
  sid=${zipfile: -11: -4}
  unzip -o "$zipfile" -d $sid > /dev/null 2>&1
  
  IFS=$'\n'
  
  filename=$(find "$sid" -type f -name "*.c")
  
  if [ -n "$filename" ];
  
  then
  searchextension $sid $filename C
  fi
  
  filename=$(find "$sid" -type f -name "*.py")
  
  if [ -n "$filename" ];
  
  then
  searchextension $sid $filename Python
  fi
  
  filename=$(find "$sid" -type f -name "*.java")
  
  if [ -n "$filename" ];
  
  then
  searchextension $sid $filename Java
  fi
  
  rm -r $sid
  
done

  
  





