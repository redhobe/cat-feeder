<template>
  <p>Настройки</p>
  <feeder-loader v-if="isLoading" />

  <ul class="feature-list">
    <li class="feature-card">
      <div class="feature-card__title">
        <i>
          <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32">
            <path
              d="M28.34 13.92c-5.26-1.22-19.54-1.17-24.67 0a.5.5 0 0 1-.6-.51c.11-2.2.43-4.42.95-6.61a2.58 2.58 0 0 1 2.06-1.96 59.8 59.8 0 0 1 5.73-.75 2.6 2.6 0 0 1 2.73 2.13c.11.57.22 1.15.35 1.73 3.8-.06 7.66.13 11.5.53a2.6 2.6 0 0 1 2.31 2.24c.11.9.2 1.8.24 2.7a.5.5 0 0 1-.6.5zM28.3 16a77.09 77.09 0 0 0-24.61 0 1.92 1.92 0 0 0-1.56 2.22c.8 5.03 2.48 7.6 3.4 8.68.36.42.83.72 1.35.87C9.66 28.6 12.83 29 16 29s6.34-.41 9.1-1.24c.53-.15 1-.45 1.35-.87.93-1.08 2.61-3.65 3.4-8.68a1.92 1.92 0 0 0-1.54-2.22z"
            />
          </svg>
        </i>
        FTP
      </div>
      <feeder-switch v-model="config.isFTPEnabled" />
    </li>
    <li class="feature-card">
      <div class="feature-card__title">
        <i>
          <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 32">
            <path
              d="M5.34 10.69q0 2.91 1.44 5.34t3.94 3.84q.51.74 1.06 1.35l-.04.06q-.04.06 0 .06v4.26q0 .9.61 1.54t1.5.6h4.3q.86 0 1.5-.6t.6-1.54v-4.32q0-.03-.03-.06.55-.61 1.1-1.35 2.42-1.37 3.9-3.84t1.47-5.34-1.44-5.35-3.9-3.9T16 0t-5.34 1.44-3.9 3.9-1.42 5.35zm8.52 20.25q0 .45.32.77t.76.29h2.12q.44 0 .76-.29t.32-.77-.32-.76-.76-.32h-2.12q-.44 0-.76.32t-.32.76z"
            />
          </svg>
        </i>
        Светодиод
      </div>
      <feeder-switch v-model="config.isLEDEnabled" />
    </li>
  </ul>
  <feeder-button text="Сохранить настройки" @click="saveConfig(config)" />
</template>

<script setup lang="ts">
import { onMounted, ref } from "vue";
import feederSwitch from "./feeder-switch.vue";
import type { IFeederConfig } from "@/types/feeder-config";
import FeederButton from "./feeder-button.vue";
import FeederLoader from "./feeder-loader.vue";
import { useConfigApi } from "@/composables/useConfigApi";

const config = ref<IFeederConfig>({
  isFTPEnabled: false,
  isLEDEnabled: false,
  schedule: [],
});
const { isLoading, getConfig, saveConfig } = useConfigApi();

onMounted(async () => (config.value = await getConfig()));
</script>

<style>
i {
  display: block;
  width: 26px;
  height: 26px;
  position: relative;
}

i svg {
  position: relative;
  z-index: 1;
}

i svg path {
  fill: var(--neutral-muted);
}
</style>
