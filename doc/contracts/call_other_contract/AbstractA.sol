pragma solidity >=0.4.0 <0.7.0;

contract AbstractA {
    function sum(uint arg1, uint arg2) returns(uint);
    // No implementation, just the function signature. This is just so Solidity can work out how to call it.
}