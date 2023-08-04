#include "PrxDescriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

// Credit to Brenden Galea for providing code for this file.

namespace prx {

    // *************** Descriptor Set Layout Builder *********************

    PrxDescriptorSetLayout::Builder& PrxDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<PrxDescriptorSetLayout> PrxDescriptorSetLayout::Builder::build() const {
        return std::make_unique<PrxDescriptorSetLayout>(prxDevice, bindings);
    }

    // *************** Descriptor Set Layout *********************

    PrxDescriptorSetLayout::PrxDescriptorSetLayout(
        PrxDevice& prxDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : prxDevice{ prxDevice }, bindings{ bindings } {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
            prxDevice.device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    PrxDescriptorSetLayout::~PrxDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(prxDevice.device(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    PrxDescriptorPool::Builder& PrxDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    PrxDescriptorPool::Builder& PrxDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    PrxDescriptorPool::Builder& PrxDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<PrxDescriptorPool> PrxDescriptorPool::Builder::build() const {
        return std::make_unique<PrxDescriptorPool>(prxDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    PrxDescriptorPool::PrxDescriptorPool(
        PrxDevice& prxDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : prxDevice{ prxDevice } {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(prxDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    PrxDescriptorPool::~PrxDescriptorPool() {
        vkDestroyDescriptorPool(prxDevice.device(), descriptorPool, nullptr);
    }

    bool PrxDescriptorPool::allocateDescriptorSet(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        // Consider using: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
        if (vkAllocateDescriptorSets(prxDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void PrxDescriptorPool::freeDescriptorSets(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            prxDevice.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void PrxDescriptorPool::resetPool() {
        vkResetDescriptorPool(prxDevice.device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    PrxDescriptorWriter::PrxDescriptorWriter(PrxDescriptorSetLayout& setLayout, PrxDescriptorPool& pool)
        : setLayout{ setLayout }, pool{ pool } {}

    PrxDescriptorWriter& PrxDescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    PrxDescriptorWriter& PrxDescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool PrxDescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void PrxDescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.prxDevice.device(), writes.size(), writes.data(), 0, nullptr);
    }

}