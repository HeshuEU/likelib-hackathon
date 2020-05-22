pragma solidity >=0.4.0 <0.7.0;

import "./abstract_a.sol";

contract A is AbstractA {
    function sum(uint arg1, uint arg2) external override returns(uint) {
        return arg1 + arg2;
    }
}
