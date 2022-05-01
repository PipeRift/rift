# Functions

## Arguments

Functions can have multiple input arguments.

Functions can also have multiple output arguments.


## Definitions

<details>
  <summary>Nodes</summary>

</details>

<details>
  <summary>Code</summary>

  ```ruby
  FillZero(U32 color) {} # An argument value
  FillOne(U32 size, U32 color = 0) {} # Multiple argument values
  FillTwo() -> Bool {} # A return value
  FillThree() -> Bool, S32 { # Multiple return values
   return true, 2;
  }
  FillFour() -> Bool valid, S32 count { # Multiple named return values
   valid = true;
   count = 2; # All named values without a default must be assigned before return.
  }

  FillFive(U32 color = 0) {} # Default argument values
  FillSix(U32 color) -> Bool=false {} # Default return values
  FillSeven() -> Bool=true, S32=undefined {} # Multiple return values with defaults
  FillEight() -> Bool valid=true, S32 count {} # Named returns with defaults
  ```
</details>


## Calls

<details>
  <summary>Nodes</summary>

</details>

<details>
  <summary>Code</summary>

  ```ruby
  FillZero(); # Error, argument must be provided
  FillZero(0);
  FillOne(0); # color is set by default to 0
  FillOne(0, 0);
  FillOne(size = 1); # Set an argument by name
  FillOne(0, color = 1);
  FillOne(size = 1, 0); # ERROR: Named arguments must be at the end of the called function.
  FillOne(color = 0, size = 1); # Set multiple arguments by name

  Bool result = FillTwo();
  [Bool result, S32 count] = FillThree(); # Define and assign returns

  Bool result; S32 count;
  [result, count] = FillThree(); # Assigning a value to existing variables
  [result, count] = FillFour(); # Named return values are returned in order

  [result, count] = FillThree(); # Named return values are returned in order
  ```
</details>


## Member Functions

<details>
  <summary>Nodes</summary>

</details>

<details>
  <summary>Code</summary>

  ```ruby
  Type one;
  one.DoSomething();

  Type* ptr = &one;
  ptr->DoSomething();
  ```
</details>


## Properties

Functions can be marked with different properties to change their behavior.
