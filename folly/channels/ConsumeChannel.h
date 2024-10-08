/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/Executor.h>
#include <folly/IntrusiveList.h>
#include <folly/channels/Channel.h>
#include <folly/channels/ChannelCallbackHandle.h>
#include <folly/coro/Task.h>
#include <folly/executors/SequencedExecutor.h>

namespace folly {
namespace channels {

/**
 * This function takes a Receiver, and consumes updates from that receiver with
 * a callback.
 *
 * This function returns a ChannelCallbackHandle. On destruction of this handle,
 * the callback will receive a try containing an exception of type
 * folly::OperationCancelled. If an active callback is running at the time the
 * cancellation request is received, cancellation will be requested on the
 * ambient cancellation token of the callback.
 *
 * The callback is run for each received value on the given executor. A try
 * is passed to the callback with the result:
 *
 *    - If a value is sent, the Try will contain the value.
 *    - If the channel is closed by the sender with no exception, the try will
 *          be empty (with no value or exception).
 *    - If the channel is closed by the sender with an exception, the try will
 *          contain the exception.
 *    - If the channel was cancelled (by the destruction of the returned
 *          handle), the try will contain an exception of type
 *          folly::OperationCancelled.
 *
 * If the callback returns false or throws a folly::OperationCancelled
 * exception, the channel will be cancelled and no further values will be
 * received.
 */
template <
    typename TReceiver,
    typename OnNextFunc,
    typename TValue = typename TReceiver::ValueType,
    std::enable_if_t<
        std::is_constructible_v<
            folly::Function<folly::coro::Task<bool>(Try<TValue>)>,
            OnNextFunc>,
        int> = 0>
ChannelCallbackHandle consumeChannelWithCallback(
    TReceiver receiver,
    folly::Executor::KeepAlive<folly::SequencedExecutor> executor,
    OnNextFunc onNext);
} // namespace channels
} // namespace folly

#include <folly/channels/ConsumeChannel-inl.h>
