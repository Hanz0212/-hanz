declare i32 @getint()          ; 读取一个整数
declare i32 @getchar()     ; 读取一个字符
declare void @putint(i32)      ; 输出一个整数
declare void @putch(i32)       ; 输出一个字符
declare void @putstr(i8*)      ; 输出字符串

@.str.1 = private unnamed_addr constant [10 x i8] c"21371295\0A\00"


define dso_local i32 @main() {
  call void @putstr(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i32 0, i32 0))
  %1 = alloca i32
  store i32 0, i32* %1
  %2 = load i32, i32* %1
  %3 = icmp sgt i32 %2, 0
  %4 = zext i1 %3 to i32
  %5 = trunc i32 %4 to i1
  br i1 %5, label %6, label %7

6:
  store i32 1, i32* %1
  br label %8

7:
  store i32 0, i32* %1
  br label %8

8:
  %9 = alloca i32
  store i32 1, i32* %9
  %10 = alloca i32
  store i32 2, i32* %10
  %11 = load i32, i32* %9
  %12 = icmp sge i32 %11, 0
  %13 = zext i1 %12 to i32
  %14 = trunc i32 %13 to i1
  br i1 %14, label %15, label %16

15:
  store i32 1, i32* %9
  br label %16

16:
  %17 = load i32, i32* %9
  %18 = icmp sge i32 %17, 0
  %19 = zext i1 %18 to i32
  %20 = trunc i32 %19 to i1
  br i1 %20, label %21, label %22

21:
  br label %22

22:
  %23 = load i32, i32* %9
  %24 = icmp ne i32 %23, 0
  %25 = zext i1 %24 to i32
  %26 = trunc i32 %25 to i1
  br i1 %26, label %27, label %28

27:
  br label %29

28:
  br label %29

29:
  %30 = load i32, i32* %10
  %31 = icmp sle i32 %30, 0
  %32 = zext i1 %31 to i32
  %33 = trunc i32 %32 to i1
  br i1 %33, label %34, label %35

34:
  store i32 2, i32* %10
  br label %35

35:
  %36 = load i32, i32* %9
  %37 = load i32, i32* %10
  %38 = icmp eq i32 %36, %37
  %39 = zext i1 %38 to i32
  %40 = trunc i32 %39 to i1
  br i1 %40, label %41, label %42

41:
  store i32 1, i32* %1
  br label %42

42:
  %43 = load i32, i32* %9
  %44 = load i32, i32* %10
  %45 = icmp ne i32 %43, %44
  %46 = zext i1 %45 to i32
  %47 = trunc i32 %46 to i1
  br i1 %47, label %48, label %49

48:
  store i32 0, i32* %1
  br label %49

49:
  %50 = load i32, i32* %9
  %51 = load i32, i32* %10
  %52 = icmp ne i32 %50, %51
  %53 = zext i1 %52 to i32
  %54 = trunc i32 %53 to i1
  br i1 %54, label %55, label %61

55:
  %56 = load i32, i32* %9
  %57 = icmp sgt i32 %56, 0
  %58 = zext i1 %57 to i32
  %59 = trunc i32 %58 to i1
  br i1 %59, label %60, label %61

60:
  store i32 1, i32* %1
  br label %61

61:
  %62 = load i32, i32* %9
  %63 = load i32, i32* %10
  %64 = icmp ne i32 %62, %63
  %65 = zext i1 %64 to i32
  %66 = trunc i32 %65 to i1
  br i1 %66, label %72, label %67

67:
  %68 = load i32, i32* %9
  %69 = icmp sgt i32 %68, 0
  %70 = zext i1 %69 to i32
  %71 = trunc i32 %70 to i1
  br i1 %71, label %72, label %73

72:
  store i32 2, i32* %1
  br label %73

73:
  %74 = load i32, i32* %9
  %75 = add i32 %74, 1
  store i32 %75, i32* %9
  store i32 2, i32* %9
  store i32 -1, i32* %9
  ret i32 0
}
