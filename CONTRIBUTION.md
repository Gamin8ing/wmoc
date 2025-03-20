
# Contributing to WMOC

Hello! 👋  
Thank you for considering contributing to **WMOC**.  
We welcome suggestions, improvements, bug reports, and pull requests!  

## How to Contribute

### 1. Raise an Issue
Before working on something, please raise an issue so we can discuss and align on solutions.  
When raising an issue, please follow this format:

#### Issue Template:
- **Issue Type**: (Bug / Feature Request / Enhancement / Question)
- **Description**:  
  Clear and concise description of the problem or suggestion.  
- **Steps to Reproduce (for bugs)**:
  1.  
  2.  
- **Expected Behavior**:
- **Actual Behavior**:
- **Possible Solutions** (if any):
- **Environment Info** (OS, compiler version, etc.):
- **Additional Context or Screenshots** (if helpful):

👉 *Example*:
> **Issue Type**: Bug  
> **Description**: Generated C code has a missing semicolon in variable declaration.  
> **Steps to Reproduce**:  
> 1. Compile a `.pl0` file with `const` section and `var` section.  
> 2. Check generated `.c` file.  
> **Expected Behavior**: Properly formed declarations with semicolons.  
> **Actual Behavior**: Semicolon missing after `int b` declaration.  
> **Environment Info**: Ubuntu 22.04, gcc 11.3.0  
> **Possible Solutions**: Add `";"` in `emitVarDecl` function.  

---

### 2. Working on an Issue
- After the issue is discussed, anyone can comment `**I’d like to work on this**` to self-assign the issue.
- Please wait for the maintainer’s confirmation before starting work.

---

### 3. Pull Requests
- Fork the repository and clone your fork.
- Work on a separate branch named after the issue number (e.g., `issue-42-fix-semicolon`).
- Follow code style that’s already used in the project.
- Test your changes with the provided test `.pl0` programs or your own.
- Once ready, raise a pull request linking to the issue (use `Closes #issue_number` in PR description).
- Add a short, clear summary in your PR explaining what’s fixed or added.

---

### 4. Code Style Guide
- Use meaningful names for variables and functions.
- Follow indentation style used in existing code.
- Keep comments concise and relevant.
- Ensure the generated C code compiles without warnings using `gcc -Wall`.

---

### 5. Communication
- Be kind and respectful in discussions.
- Ask if you’re unsure!  
- Everyone is welcome to contribute — whether it's your first PR or your hundredth!

---

## Thank You!
Your contributions make this project better for everyone — and help us all learn compiler design together! ❤️

