
const HOUR_UNIT: number = 60;

const TIME_CONST_TEN: number = 10;

export function secondToTime(seconds: number): string {
  let hourUnit: number = HOUR_UNIT * HOUR_UNIT;
  let hour: number = Math.floor(seconds / hourUnit);
  let minute: number = Math.floor((seconds - hour * hourUnit) / HOUR_UNIT);
  let second: number = seconds - hour * hourUnit - minute * HOUR_UNIT;
  let hourStr: string = hour < TIME_CONST_TEN ? `0${hour.toString()}` : `${hour.toString()}`
  let minuteStr: string = minute < TIME_CONST_TEN ? `0${minute.toString()}` : `${minute.toString()}`
  let secondStr: string = second < TIME_CONST_TEN ? `0${second.toString()}` : `${second.toString()}`
  if (hour > 0) {
    return `${hourStr}:${minuteStr}:${secondStr}`;
  }
  if (minute > 0) {
    return `${minuteStr}:${secondStr}`;
  } else {
    return `00:${secondStr}`;
  }
}