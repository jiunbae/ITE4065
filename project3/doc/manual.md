## Wait-Free Snapshot 구현하기

- Update 와 Scan 인터페이스 C/C++ 로 구현하기
- 각 Thread 는 자신의 로컬 변수에 대해서 Update 를 수행한다.
- Thread 개수는 실행 시 파라미터로 받아서 조절한다.
- 1분동안 모든 쓰레드가 수행한 update 의 개수를 측정한다.
- 쓰레드의 개수를 1, 2, 4, 8, 16, 32 로 증가시켜보면서 측정되는 update 의 개수를 그래프로 작성해서 wiki 에 보고서를 작성한다.
- hconnect의 개인 repository에서 project3 디렉토리가 있으며 이 위치에서 make가 실행되고 그 결과로 "run"이름의 실행파일이 생성되어야 한다.

**Please document the assignment's manual. The assignment description written in the post is cumbersome to check, and it is difficult to explicitly confirm changes to the assignment.**