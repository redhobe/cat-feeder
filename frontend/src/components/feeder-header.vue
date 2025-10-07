<template>
  <feeder-loader v-if="isLoading" />
  <div class="main-container__left">
    <div class="main-img">
      <img src="../assets/petkit.webp" alt="logo" />
    </div>
  </div>
  <div class="main-container__right">
    <h1>PETKIT FRESH ELEMENT SOLO</h1>
    <div class="feature-list">
      <p>Количество порций</p>
      <feeder-input
        v-model="portionsCount"
        type="number"
        pattern="\d*"
        min="1"
        max="10"
        placeholder="Введите кол-во"
        style="width: 100%"
      />
      <feeder-button text="Покормить" @click="onFeedButtonClick" />
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from "vue";
import FeederInput from "./feeder-input.vue";
import FeederButton from "./feeder-button.vue";
import FeederLoader from "./feeder-loader.vue";

const portionsCount = ref<string>("3");
const isLoading = ref<boolean>(false);

async function onFeedButtonClick() {
  try {
    isLoading.value = true;
    await fetch(`/feed?portions=${portionsCount.value}`);
    alert(`Порции: ${portionsCount.value}`);
  } catch (error) {
    alert(error);
  } finally {
    isLoading.value = false;
  }
}
</script>

<style>
.main-container {
  padding: 40px;
  gap: 40px;
  justify-content: center;
}

.main-container__left {
  position: relative;
}

.main-img {
  position: relative;
  border-radius: 20px;
  overflow: hidden;
}

.main-container__right {
  display: flex;
  flex-direction: column;
}

h1 {
  font-size: 28px;
}

.main-img .main-img img {
  position: relative;
  width: 100%;
  height: 100%;
}
</style>
