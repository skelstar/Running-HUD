// void peekQueue()
// {
//     CommandPacket *commandPacket = nullptr;
//     if (xQueuePeek(xCommandQueue, &(commandPacket), TICKS_50ms) == pdTRUE &&
//         commandPacket->id != commandPacketId)
//     {
//         // missed a packet
//         if (commandPacket->id - commandPacketId > 1)
//         {
//             Serial.printf("Command Rx id: %lu (commandPacketId: %lu) \n",
//                           commandPacket->id, commandPacketId);
//         }
//         commandPacketId = commandPacket->id;

//         handleCommandPacket(commandPacket);
//     }
//     vTaskDelay(TICKS_50ms);
// }