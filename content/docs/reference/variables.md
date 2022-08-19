---
title: "Variables"
draft: false
toc: true
---

A variable is a block of memory containing an struct, a class, a native type, pointers or function pointers.

## Initialization

Variables are always default-initialized unless marked as **undefined** which keeps the memory as it was before initialization (garbage memory).

<details>
  <summary>Nodes</summary>

</details>

<details>
  <summary>Code</summary>

  ```ruby
  struct Type
  {
      Bool alive;
      Bool running;
  }

  void main()
  {
      Type one; // Default initialized
      Type two = undefined; // Not initialized (garbage)
      Type three(true); // 'alive' set to true
      Type four(running=true); // 'running' set to true

      Type five = four; // Copy four into five
      Type six := four; // Move four into six
      Type* seven = &six; // Seven is pointer to four
  }
  ```
</details>

## Member Variables
<details>
  <summary>Nodes</summary>

</details>

<details>
  <summary>Code</summary>

  ```ruby
  Type one;
  one.alive = true;

  Type* ptr = &one;
  ptr->alive = false;
  ```
</details>