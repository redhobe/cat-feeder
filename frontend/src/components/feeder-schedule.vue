<template>
  <p>Расписание</p>

  <feeder-loader v-if="isLoading" />
  <table align="left">
    <thead>
      <tr>
        <th>Время</th>
        <th>Порции</th>
      </tr>
    </thead>
    <tbody>
      <tr v-for="(row, index) in config?.schedule" :key="index">
        <td><feeder-input type="time" v-model="row.time" /></td>
        <td style="width: 100px">
          <feeder-input
            type="number"
            pattern="\d*"
            v-model="row.portions"
            min="1"
            max="10"
            style="width: 100px"
          />
        </td>
        <td style="width: 100%">
          <feeder-button round theme="error" @click="deleteFromSchedule(index)">
            ✕
          </feeder-button>
        </td>
      </tr>
    </tbody>
  </table>

  <p style="font-size: 14px">Добавить запись</p>

  <div style="display: flex; flex-wrap: nowrap; gap: 16px; align-items: end">
    <div class="form-item">
      <label for="time">Время</label>
      <feeder-input v-model="addTableRowTime" type="time" placeholder="--:--" />
    </div>

    <div class="form-item">
      <label for="portions">Кол-во порций</label>
      <feeder-input
        v-model="addTableRowPortions"
        style="width: 160px"
        type="number"
        pattern="\d*"
        min="1"
        max="10"
      />
    </div>
    <feeder-button round theme="success" @click="addToSchedule">
      ＋
    </feeder-button>
  </div>
  <feeder-button text="Сохранить расписание" @click="saveConfig(config)" />
</template>

<script setup lang="ts">
import { onMounted, ref } from "vue";
import feederInput from "./feeder-input.vue";
import FeederButton from "./feeder-button.vue";
import FeederLoader from "./feeder-loader.vue";
import type { IFeederConfig } from "@/types/feeder-config";
import { useConfigApi } from "@/composables/useConfigApi";

const addTableRowTime = ref<string>("12:00");
const addTableRowPortions = ref<string>("3");
const config = ref<IFeederConfig>({
  isFTPEnabled: false,
  isLEDEnabled: false,
  schedule: [],
});
const { isLoading, getConfig, saveConfig } = useConfigApi();

onMounted(async () => (config.value = await getConfig()));

function addToSchedule() {
  config.value?.schedule.push({
    time: addTableRowTime.value,
    portions: Number(addTableRowPortions.value),
  });
}

function deleteFromSchedule(index: number) {
  config.value?.schedule.splice(index, 1);
}
</script>

<style>
table {
  border-collapse: collapse;
  width: 100%;
  border-radius: var(--radius-md);
  font-size: 12px;
  overflow: hidden;
}

table:not(:has(tbody tr)) {
  display: none;
}

th {
  font-weight: 500;
}

th,
td {
  text-align: left;
  padding: 16px;
}

td:nth-of-type(3) {
  text-align: end;
}

tbody > tr:nth-of-type(odd) {
  background: var(--bg-surface-3);
}

.form-item {
  display: flex;
  flex-direction: column;
}

.form-item label {
  cursor: text;
  z-index: 2;
  font-size: 12px;
  font-weight: regular;
  background: #fff;
  padding: 4px 0;
  color: #999;
  transition: all 0.3s ease;
}

.form-item input:focus + label,
.form-item input:valid + label {
  font-size: 11px;
  top: -5px;
}
</style>
