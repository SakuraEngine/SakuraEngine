#pragma once

// TODO. 内置 RC 接口，提供 RC 和 WRC 两种类型的指针
//  RC：直接操作 ref_count，ref_count 减为 0 时销毁
//  WRC：通过一个 counter_object 间接访问 RC，使用时需要 lock 成 RC
//  URC：行为近似 unique_ptr，只能通过 move 转移其所有权
//  Pooling：通过允许 RC 对象主动提供 deleter 的方式实现
//  OwnerShip：我们不希望 RC 被滥用，到处传递，我们希望程序员明确对象所有权，在大多数环境下使用 WRC 来保持引用
//      因此，需要引入约束的方式，主要又两种：
//      1. add_reference 的时候，提供一个 AddReason，用于检查不当的引用增加
//      2. 在主权决定销毁对象时，如果计数不为 1，则直接报错，或添加到一个监视器中，监视僵尸对象