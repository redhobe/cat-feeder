<template>
  <div class="feeder-main-page">
    <div class="container main-container">
      <feeder-header />
    </div>
    <div class="container">
      <feeder-schedule />
    </div>
    <!-- <div class="container">
      <feeder-settings />
    </div> -->
  </div>
</template>

<script setup lang="ts">
import FeederHeader from "./components/feeder-header.vue";
import FeederSchedule from "./components/feeder-schedule.vue";
</script>

<style>
:root {
  --primary-default: #147efb;
  --content-default: #1f1f21;
  --bg-surface-1: #ffffff;
  --bg-surface-2: #f7f7f7;
  --bg-surface-3: #f6f6f8;
  --neutral-container-default: #4453711a;
  --neutral-muted: #bdbec2;
  --border-muted: #4453711a;
  --border-light: #44537133;
  --radius-md: 22px;
  --radius-lg: 22px;
  --success-default: #4cd964;
  --error-default: #ff3a2d;
  --error-bg: #ffd3e0;
  --font-family: "Golos Text", sans-serif;
}

p {
  margin: 0;
  width: 100%;
  font-size: 16px;
  font-weight: 500;
}

* {
  box-sizing: border-box;
  font-family: var(--font-family);
}

body {
  margin: 0;
  padding: 0;
  background-color: var(--bg-surface-2);
  font-family: "Golos Text", sans-serif;
}

::-webkit-scrollbar {
  /*Chrome, Safari, Edge*/
  -webkit-appearance: none;
  display: none;
  width: 0;
  height: 0;
}

.feeder-main-page {
  max-width: 820px;
  margin: auto;
  padding: 16px;
  height: 100%;
  gap: 16px;
  flex-wrap: wrap;
  display: flex;
  justify-content: center;
  font-weight: 400;
  color: var(--content-default);
}

.container {
  border: 1px solid var(--border-muted);
  background-color: var(--bg-surface-1);
  width: 100%;
  display: flex;
  flex-wrap: wrap;
  overflow: hidden;
  box-sizing: border-box;
  padding: 20px 24px;
  gap: 20px;
  border-radius: var(--radius-lg);
  position: relative;
}
.feature-list {
  list-style: none;
  padding: 0;
  margin: 0;
  display: flex;
  flex-direction: row;
  flex-wrap: wrap;
  gap: 10px;
}

.feature-card {
  padding: 8px 8px 8px 16px;
  border-radius: 40px;
  border: 1px solid var(--border-muted--border-muted--border-muted);
  background: var(--neutral-container-default);
  font-size: 16px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  box-shadow: 8px 40px 96px 0px #123d6524, -8px -6px 80px 0px #ffffff2e inset;
}

.feature-card__title {
  display: flex;
  align-items: center;
  gap: 10px;
}
</style>
