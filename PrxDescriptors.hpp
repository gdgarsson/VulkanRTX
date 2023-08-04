#pragma once

#include "PrxDevice.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

// Credit to Brenden Galea for providing code for this file.

namespace prx {

    class PrxDescriptorSetLayout {
    public:
        class Builder {
        public:
            // Note: this class is meant to help make constructing the unordered map easier
            Builder(PrxDevice& prxDevice) : prxDevice{ prxDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<PrxDescriptorSetLayout> build() const;

        private:
            PrxDevice& prxDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        PrxDescriptorSetLayout(
            PrxDevice& prxDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~PrxDescriptorSetLayout();
        PrxDescriptorSetLayout(const PrxDescriptorSetLayout&) = delete;
        PrxDescriptorSetLayout& operator=(const PrxDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        PrxDevice& prxDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class PrxDescriptorWriter;
    };

    class PrxDescriptorPool {
    public:
        class Builder {
        public:
            Builder(PrxDevice& prxDevice) : prxDevice{ prxDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<PrxDescriptorPool> build() const;

        private:
            PrxDevice& prxDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        PrxDescriptorPool(
            PrxDevice& prxDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~PrxDescriptorPool();
        PrxDescriptorPool(const PrxDescriptorPool&) = delete;
        PrxDescriptorPool& operator=(const PrxDescriptorPool&) = delete;

        bool allocateDescriptorSet(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptorSets(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        PrxDevice& prxDevice;
        VkDescriptorPool descriptorPool;

        friend class PrxDescriptorWriter;
    };

    class PrxDescriptorWriter {
    public:
        PrxDescriptorWriter(PrxDescriptorSetLayout& setLayout, PrxDescriptorPool& pool);

        PrxDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        PrxDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        PrxDescriptorSetLayout& setLayout;
        PrxDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

} 