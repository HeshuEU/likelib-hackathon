pragma solidity >=0.4.0 <0.8.5;

contract GetCode {
    function getAddressBytes(address _addr) public view returns (bytes memory o_code) {
        assembly {
            let size := extcodesize(_addr)
            o_code := mload(0x40)
            mstore(0x40, add(o_code, and(add(add(size, 0x20), 0x1f), not(0x1f))))
            mstore(o_code, size)
            extcodecopy(_addr, add(o_code, 0x20), 0, size)
        }
    }
}
