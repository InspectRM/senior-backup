#Double Fork Pattern
Why MS_PRIVATE matters:

-Prevents container mounts from appearing on host
-Prevents host mounts from appearing in container
-Critical for true isolation

Process hierarchy:

```
Original Process (Parent)
  └── Child (enters namespaces)
      └── Grandchild (PID 1 in container, runs program)
```