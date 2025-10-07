<template>
  <label class="toggleSwitch" onclick="">
    <input type="checkbox" v-model="model" />
    <span>
      <span>ВЫКЛ</span>
      <span>ВКЛ</span>
    </span>
    <a></a>
  </label>
</template>

<script setup lang="ts">
const model = defineModel<boolean>();
</script>

<style>
/*  Toggle Switch  */

.toggleSwitch span span {
  display: none;
}

.toggleSwitch {
  display: inline-block;
  height: 36px;
  position: relative;
  overflow: visible;
  padding: 0;
  margin-left: 50px;
  cursor: pointer;
  width: 83px;
  background: linear-gradient(135deg, #e7eff5 0%, #ffffff 100%);
  border: 2px solid #cfdbe7;
  border-radius: 100px;
  box-shadow: 0px 1px 9px 0px #c8d4e2 inset, -5px -4px 5px 0px #ffffffbf inset;
}

.toggleSwitch * {
  -webkit-box-sizing: border-box;
  -moz-box-sizing: border-box;
  box-sizing: border-box;
}

.toggleSwitch > span {
  line-height: 20px;
  height: 14px;
}

.toggleSwitch input {
  position: absolute;
  opacity: 0;
  z-index: 5;
}

.toggleSwitch > span {
  position: absolute;
  width: 100%;
  color: #31456a;
  opacity: 0.6;
  transition: all 0.3s ease;
  user-select: none;
}

.toggleSwitch > span span {
  position: absolute;
  display: block;
  font-size: 12px;
  opacity: 0;
  top: 50%;
  bottom: 50%;
  margin: auto 0;
}

.toggleSwitch a {
  position: absolute;
  right: 50%;
  z-index: 4;
  display: block;
  left: 10px;
  height: 18px;
  width: 18px;
  top: 50%;
  bottom: 50%;
  margin: auto 0;
  border-radius: 100%;
  background: linear-gradient(89.91deg, #ff6b59 -8.26%, #d35646 144.55%);
  box-shadow: 6px 9px 40px 0px #ff8677, -6px -6px 16px 0px #ffffff99;
  transition: all 0.2s ease-out;
}

.toggleSwitch > span span:first-of-type {
  opacity: 1;
  left: 45%;
}

.toggleSwitch input:checked ~ a {
  left: 100%;
  margin-left: -28px;
  background: linear-gradient(89.91deg, #59ff7e -8.26%, #46d365 144.55%);
  box-shadow: 6px 9px 40px 0px #77ff85, -6px -6px 16px 0px #ffffff99;
}

.toggleSwitch input:checked ~ span span:first-of-type {
  transition: all 0.2s ease-out;
  opacity: 0;
}

.toggleSwitch input:checked ~ span span:last-of-type {
  opacity: 1;
  transition: all 0.2s ease-out;
  left: 10px;
}
</style>
